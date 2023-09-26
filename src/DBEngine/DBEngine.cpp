#include <algorithm>
#include <bitset>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>

#include <spdlog/spdlog.h>
#include <variant>

#include "Constants.hpp"
#include "DBEngine.hpp"
#include "Record/Record.hpp"

using DB_ENGINE::DBEngine;

using ::std::filesystem::create_directory;
using ::std::filesystem::exists;

constexpr float FLOAT_EPSILON = 0.001F;

static auto get_sample_value(DB_ENGINE::Type type) -> std::string {
  switch (type.type) {
  case DB_ENGINE::Type::BOOL:
    return "true";
  case DB_ENGINE::Type::INT:
    return "0";
  case DB_ENGINE::Type::FLOAT:
    return "0.0";
  case DB_ENGINE::Type::VARCHAR:
    return "a";
  }
  throw std::runtime_error("Invalid type");
}

DB_ENGINE::DBEngine::DBEngine() {
  generate_directories();

  // Read tables raw data
  for (auto const &dir_entry :
       std::filesystem::directory_iterator{TABLES_PATH}) {
    spdlog::info(dir_entry.path().string());
    spdlog::info(dir_entry.path().filename().string());
    HeapFile heap_file(dir_entry.path());
    m_tables_raw.insert(
        {dir_entry.path().filename().string(), std::move(heap_file)});
  }

  // Load Indexes
}

auto DBEngine::create_table(const std::string &table_name,
                            const std::string &primary_key,
                            const std::vector<Type> &types,
                            const std::vector<std::string> &attribute_names)
    -> bool {

  // Check if table already exists
  std::string table_path = TABLES_PATH + table_name;

  if (exists(table_path)) {
    spdlog::warn("Table {} already exists doing nothing.", table_name);
    return false;
  }

  spdlog::info("Creating table {} in dir {}", table_name, table_path);

  create_directory(table_path);

  HeapFile heap_file(table_name, types, attribute_names, primary_key);

  m_tables_raw.insert({table_name, std::move(heap_file)});

  auto key_type = m_tables_raw.at(table_name).get_type(primary_key);

  switch (key_type.type) {
  case Type::INT: {
    SequentialIndex<int> sequential_index_int(table_name, primary_key, true);
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_int)});
    break;
  }
  case Type::FLOAT: {
    SequentialIndex<float> sequential_index_float(table_name, primary_key,
                                                  true);
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_float)});
    break;
  }
  case Type::VARCHAR: {
    spdlog::error("Varchar can't be indexed. at: table_creation");
    throw std::invalid_argument("Varchar can't be indexed");
  }
  case Type::BOOL: {
    spdlog::error("Bool can't be indexed. at: table_creation");
    throw std::invalid_argument("Bool can't be indexed");
  }
  }

  return true;
}

auto DBEngine::get_table_names() -> std::vector<std::string> {

  std::vector<std::string> table_names;

  for (const auto &table : m_tables_raw) {
    table_names.push_back(table.first);
  }
  return table_names;
}

auto DBEngine::search(const std::string &table_name, const Attribute &key,
                      const std::function<bool(Record)> &expr,
                      const std::vector<std::string> &selected_attributes)
    -> QueryResponse {

  std::vector<HeapFile::pos_type> positions;

  query_time_t times;

  auto key_type = m_tables_raw.at(table_name).get_type(key);

  key_cast_and_execute(
      key_type.type,
      [this, &key, &times, &positions](ValidIndexType auto key_value) {
        for (const auto &idx : m_sequential_indexes) {
          if (idx.second.get_attribute_name() == key.name) {

            auto search_response = idx.second.search(key_value);
            positions = search_response.first;
            times["SEQUENTIAL_SINGLE_SEARCH"] = search_response.second;
            break;
          }
        }
      },
      key.value);

  auto heap_response = m_tables_raw.at(table_name).read(positions);
  times["HEAP_BULK_READ"] = heap_response.second;

  heap_response.first.erase(
      std::remove_if(heap_response.first.begin(), heap_response.first.end(),
                     [&expr](const auto &record) { return !expr(record); }),
      heap_response.first.end());

  return m_tables_raw.at(table_name)
      .filter(heap_response.first, selected_attributes, times);
}
auto DBEngine::range_search(const std::string &table_name, Attribute begin_key,
                            Attribute end_key,
                            const std::function<bool(Record)> &expr,
                            const std::vector<std::string> &selected_attributes)
    -> QueryResponse {

  query_time_t times;

  if (begin_key == KEY_LIMITS::MIN && end_key == KEY_LIMITS::MAX) {
    QueryResponse response = load(table_name, selected_attributes, times);

    response.records.erase(
        std::remove_if(response.records.begin(), response.records.end(),
                       [&expr](const Record &obj) { return !expr(obj); }),
        response.records.end());
  }

  if (begin_key.name != end_key.name) {

    if ((end_key != KEY_LIMITS::MIN && end_key != KEY_LIMITS::MAX) &&
        (begin_key != KEY_LIMITS::MIN && begin_key != KEY_LIMITS::MAX)) {
      throw std::runtime_error(
          "Cant apply range_search to different attributes");
    }
  }

  if (begin_key == KEY_LIMITS::MIN) {
    begin_key.value = std::to_string(std::numeric_limits<int>::min());
    begin_key.name = end_key.name;
  }
  if (end_key == KEY_LIMITS::MAX) {
    end_key.value = std::to_string(std::numeric_limits<int>::max());
    end_key.name = begin_key.name;
  }
  spdlog::info("Range search names: {}, {}", begin_key.name, end_key.name);
  spdlog::info("Range search values: {}, {}", begin_key.value, end_key.value);

  std::vector<HeapFile::pos_type> positions;

  auto key_type = m_tables_raw.at(table_name).get_type(begin_key);

  key_cast_and_execute(
      key_type.type,
      [this, &positions, &begin_key,
       &times](ValidIndexType auto begin_key_value,
               ValidIndexType auto end_key_value) {
        for (const auto &idx : m_sequential_indexes) {
          if (idx.second.get_attribute_name() == begin_key.name) {
            auto idx_response =
                idx.second.range_search(begin_key_value, end_key_value);
            positions = idx_response.records;
            times["SEQUENTIAL_RANGE_SEARCH"] = idx_response.query_time;
          }
        }
      },
      begin_key.value, end_key.value);

  auto heap_response = m_tables_raw.at(table_name).read(positions);
  times["HEAP_BULK_READ"] = heap_response.second;

  heap_response.first.erase(
      std::remove_if(heap_response.first.begin(), heap_response.first.end(),
                     [&expr](const auto &record) { return !expr(record); }),
      heap_response.first.end());

  return m_tables_raw.at(table_name)
      .filter(heap_response.first, selected_attributes, times);
}

auto DBEngine::load(const std::string &table_name,
                    const std::vector<std::string> &selected_attributes)
    -> QueryResponse {
  query_time_t times;
  return load(table_name, selected_attributes, times);
}

auto DBEngine::load(const std::string &table_name,
                    const std::vector<std::string> &selected_attributes,
                    query_time_t &times) -> QueryResponse {

  auto response = m_tables_raw.at(table_name).load();

  times["HEAP_FILE_LOAD"] = response.second;

  return m_tables_raw.at(table_name)
      .filter(response.first, selected_attributes, times);
}

auto DBEngine::add(const std::string &table_name,
                   const std::vector<std::string> &value) -> bool {

  spdlog::info("Calling add");

  Record rec(value);

  auto &table = m_tables_raw.at(table_name);

  auto [type, key] = table.get_key(rec);

  auto inserted_pos = table.next_pos();

  response_time time;

  bool pk_inserted = false;

  // Insert into primery key index

  auto idx = m_sequential_indexes.at({table_name, key.name});
  key_cast_and_execute(
      type.type,
      [&idx, &inserted_pos, &pk_inserted, &time](ValidIndexType auto key_val) {
        auto add_response = idx.add(key_val, inserted_pos);
        pk_inserted = add_response.first;
        time = add_response.second;
      },
      key.value);

  if (!pk_inserted) {
    spdlog::info("Record with key {} already exists", key.name);
    throw std::runtime_error("Record already exists");
  }

  // Insert whole record
  m_tables_raw.at(table_name).add(rec);

  const auto &types = m_tables_raw.at(table_name).get_types();
  const auto &attribute_names =
      m_tables_raw.at(table_name).get_attribute_names();

  // Insert into all the other available indexes
  for (ulong curr_field = 0; const auto &field_str_value : rec.m_fields) {

    auto attribute_name = attribute_names.at(curr_field);
    auto attribute_type = types.at(curr_field).type;

    if (attribute_type != Type::INT && attribute_type != Type::FLOAT) {
      curr_field++;
      continue;
    }

    key_cast_and_execute(
        attribute_type,
        [&table_name, &attribute_name, this, &key,
         &inserted_pos](ValidIndexType auto casted_field) {
          using field_type = decltype(casted_field);

          if (m_sequential_indexes.contains({table_name, attribute_name}) &&
              attribute_name != key.name) {
            m_sequential_indexes.at({table_name, attribute_name})
                .add<field_type>(casted_field, inserted_pos);
          }
          if (m_avl_indexes.contains({table_name, attribute_name})) {
            m_avl_indexes.at({table_name, attribute_name})
                .add<field_type>(casted_field, inserted_pos);
          }
          if (m_isam_indexes.contains({table_name, attribute_name})) {
            m_isam_indexes.at({table_name, attribute_name})
                .add<field_type>(casted_field, inserted_pos);
          }
        },
        field_str_value);
    curr_field++;
  }
  return pk_inserted;
}

auto DBEngine::remove(const std::string &table_name,
                      const Attribute &key_str_value)
    -> std::pair<query_time_t, bool> {

  auto [key_type, key_name] = m_tables_raw.at(table_name).get_key_name();

  query_time_t times;
  HeapFile::pos_type raw_pos;

  auto idx = m_sequential_indexes.at({table_name, key_name});

  key_cast_and_execute(
      key_type.type,
      [&raw_pos, &idx, &times](ValidIndexType auto key_value) {
        auto remove_response = idx.remove(key_value);
        raw_pos = remove_response.first;
        times["REMOVE_FROM_PK"] = remove_response.second;
      },
      key_str_value.value);

  if (raw_pos == -1) {
    return {times, false};
  }

  m_tables_raw.at(table_name).remove(raw_pos);

  // Remove from other indexes
  throw std::runtime_error("Not yet implemented");

  return {times, true};
}

DB_ENGINE::query_time_t DBEngine::create_index(const std::string &table_name,
                                               const std::string &column_name,
                                               const Index_t &index_type) {

  query_time_t times;

  auto record_size = m_tables_raw.at(table_name).get_record_size();
  auto pos_getter = [&record_size]() {
    // Starting pos
    static std::streampos prev_pos = -record_size;
    prev_pos += record_size;
    return prev_pos;
  };

  // Early return clause
  switch (index_type) {
  case Index_t::AVL: {
    if (m_avl_indexes.contains({table_name, column_name})) {
      spdlog::warn("Index already exists");
      throw std::invalid_argument("Index already exists");
    }
    break;
  }
  case Index_t::ISAM: {
    if (m_isam_indexes.contains({table_name, column_name})) {
      spdlog::warn("Index already exists");
      throw std::invalid_argument("Index already exists");
    }
    break;
  }
  case Index_t::SEQUENTIAL: {
    if (m_sequential_indexes.contains({table_name, column_name})) {
      spdlog::warn("Index already exists");
      throw std::invalid_argument("Index already exists");
    }
    break;
  }
  }

  const auto TYPE = m_tables_raw.at(table_name).get_type(column_name);

  if (TYPE.type == Type::BOOL) {
    spdlog::error("Bool can't be indexed. at: create_index");
    throw std::invalid_argument("Bool can't be indexed");
  }

  key_cast_and_execute(
      TYPE.type,
      [&index_type, &table_name, &column_name, this, &TYPE, &pos_getter,
       &times](ValidIndexType auto value) {
        using att_type = decltype(value);

        std::vector<std::pair<att_type, std::streampos>> key_values;
        auto load_response = load(table_name, {column_name}, times);

        auto all_records = load_response.records;
        key_values.reserve(all_records.size());

        auto start_time = std::chrono::high_resolution_clock::now();

        std::transform(
            all_records.begin(), all_records.end(),
            std::back_inserter(key_values), [&TYPE, &pos_getter](auto record) {
              auto str_val = record.m_fields.at(0);
              auto str_value = std::string(str_val.begin(), str_val.end());

              std::pair<att_type, std::streampos> return_v;

              switch (TYPE.type) {

              case Type::BOOL:
                spdlog::error("Bool can't be indexed. at: create_index");
                throw std::invalid_argument("Bool can't be indexed");
              case Type::VARCHAR:
                spdlog::error("VARCHAR can't be indexed. at: create_index");
                throw std::invalid_argument("VARCHAR can't be indexed");
                break;

              case Type::INT:
                return_v = {std::stoi(str_value), pos_getter()};
                break;

              case Type::FLOAT:
                return_v = {std::stof(str_value), pos_getter()};
                break;
              }
              return return_v;
            });
        auto end_time = std::chrono::high_resolution_clock::now();

        times["GENERATE_VEC_OF_PAIRS"] =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                  start_time);

        switch (index_type) {
        case Index_t::AVL: {

          AVLIndex<att_type> idx(table_name, column_name, false);
          m_avl_indexes.emplace(Index(table_name, column_name), idx);
          times["AVL_BULK_INSERTION"] =
              m_avl_indexes.at({table_name, column_name})
                  .bulk_insert<att_type>(key_values)
                  .first.query_time;
          break;
        }
        case Index_t::SEQUENTIAL: {

          SequentialIndex<att_type> idx(table_name, column_name, false);
          m_sequential_indexes.emplace(Index(table_name, column_name), idx);
          times["SEQUENTIAL_BULK_INSERTION"] =
              m_sequential_indexes.at({table_name, column_name})
                  .bulk_insert<att_type>(key_values)
                  .first.query_time;
          break;
        }
        case Index_t::ISAM: {
          SequentialIndex<att_type> idx(table_name, column_name, false);
          m_isam_indexes.emplace(Index(table_name, column_name), idx);
          times["ISAM_BULK_INSERTION"] =
              m_isam_indexes.at({table_name, column_name})
                  .bulk_insert<att_type>(key_values)
                  .first.query_time;

          break;
        }
        }
      },
      get_sample_value(TYPE));
  return times;
}

enum class INDEX_OPTION : uint8_t {
  NONE = 0U,
  AVL = 1U << 0U,
  ISAM = 1U << 1U,
  SEQUENTIAL = 1U << 2U
};
static auto operator<<(std::ostream &ost, const INDEX_OPTION &option)
    -> auto & {

  switch (option) {

  case INDEX_OPTION::NONE:
    return ost << "NONE";
  case INDEX_OPTION::AVL:
    return ost << "AVL";
  case INDEX_OPTION::ISAM:
    return ost << "ISAM";
  case INDEX_OPTION::SEQUENTIAL:
    return ost << "SEQUENTIAL";
  }

  return ost << static_cast<uint8_t>(option);
}

constexpr auto operator+=(INDEX_OPTION &lhs, const INDEX_OPTION &rhs)
    -> INDEX_OPTION & {
  lhs = static_cast<INDEX_OPTION>(static_cast<uint8_t>(lhs) |
                                  static_cast<uint8_t>(rhs));
  return lhs;
}

struct IndexInsert {

  IndexInsert() = default;

  template <ValidType T>
  IndexInsert(const std::vector<T> &_values) : values(_values) {
    if (std::is_same_v<std::vector<int>, decltype(_values)>) {
    }
  }

  template <ValidType T, class... Args> void emplace_back(Args &&...args) {
    std::get<std::vector<T>>(values).emplace_back(std::forward<Args>(args)...);
  }
  template <ValidType T> void reserve(const auto &reserve_size) {
    std::get<std::vector<T>>(values).reserve(reserve_size);
  }
  template <ValidType T> [[nodiscard]] auto at(const auto &idx) const -> T {
    return std::get<std::vector<T>>(values).at(idx);
  }
  template <ValidType T> [[nodiscard]] auto size() const -> ulong {
    return std::get<std::vector<T>>(values).size();
  }

  std::variant<std::vector<int>, std::vector<float>> values;
};

template <ValidType T>
auto handle_option(const INDEX_OPTION &option, const IndexInsert &inserted_keys,
                   const std::vector<bool> &pk_insertion,
                   std::optional<AvlIndexContainer> avl, auto sequential,
                   auto isam, const auto &get_pos,
                   DB_ENGINE::query_time_t &times) -> void {

  if (option == INDEX_OPTION::NONE) {
    return;
  }

  std::bitset<3> options(static_cast<uint8_t>(option));

  std::vector<std::pair<T, std::streampos>> inserted_indexes;
  inserted_indexes.reserve(inserted_keys.size<T>());

  for (ulong i = 0; i < inserted_keys.size<T>(); i++) {
    if (pk_insertion.at(i)) {
      inserted_indexes.emplace_back(
          std::make_pair(inserted_keys.at<T>(i), get_pos()));
    }
  }

  std::jthread avl_insert;
  if (options.test(0)) {
    spdlog::info("Found avl");
    avl_insert = std::jthread([&avl, &inserted_indexes, &times]() {
      times["AVL_BULK_INSERTION"] =
          avl.value()
              .template bulk_insert<T>(inserted_indexes)
              .first.query_time;
    });
  }
  std::jthread isam_insert;
  if (options.test(1)) {
    spdlog::info("Found isam");
    isam_insert = std::jthread([&isam, &inserted_indexes, &times]() {
      times["AVL_BULK_INSERTION"] =
          isam.value()
              .template bulk_insert<T>(inserted_indexes)
              .first.query_time;
    });
  }
  std::jthread sequential_insert;
  if (options.test(2)) {
    spdlog::info("Found seq");
    sequential_insert =
        std::jthread([&sequential, &inserted_indexes, &times]() {
          times["AVL_BULK_INSERTION"] =
              sequential.value()
                  .template bulk_insert<T>(inserted_indexes)
                  .first.query_time;
        });
  }

  avl_insert.join();
  isam_insert.join();
  sequential_insert.join();
}

static auto generate_key_output(const std::vector<DB_ENGINE::Type> &field_types,
                                const ulong &min_record_count)
    -> std::vector<IndexInsert> {

  std::vector<IndexInsert> inserted_keys;
  inserted_keys.resize(field_types.size());

  for (ulong idx = 0; const auto &idx_type : field_types) {
    switch (idx_type.type) {
    case DB_ENGINE::Type::INT: {

      IndexInsert insert_vec((std::vector<int>()));
      inserted_keys.at(idx) = insert_vec;
      inserted_keys.at(idx).reserve<int>(min_record_count);
      break;
    }
    case DB_ENGINE::Type::FLOAT: {
      IndexInsert insert_vec((std::vector<float>()));
      inserted_keys.at(idx) = insert_vec;
      inserted_keys.at(idx).reserve<float>(min_record_count);
      break;
    }
    case DB_ENGINE::Type::VARCHAR:
    case DB_ENGINE::Type::BOOL:
      break;
    }
    idx++;
  }
  return inserted_keys;
}

void insert_field(const auto &field, const ulong &curr_field,
                  const std::vector<DB_ENGINE::Type> &field_types,
                  std::vector<IndexInsert> &inserted_keys) {

  switch (field_types.at(curr_field).type) {
  case DB_ENGINE::Type::INT: {
    auto field_val = std::string(field.begin(), field.end());

    inserted_keys.at(curr_field)
        .template emplace_back<int>(std::stoi(field_val));
    break;
  }

  case DB_ENGINE::Type::FLOAT: {

    inserted_keys.at(curr_field)
        .template emplace_back<float>(
            std::stof(std::string(field.begin(), field.end())));
    break;
  }
  case DB_ENGINE::Type::VARCHAR:
  case DB_ENGINE::Type::BOOL:
    spdlog::info("a value was not inserted(VCH or BOOL)");
    break;
  }
}

template <typename T>
auto get_idx(std::string table_name, std::string attribute_name, T idx_map)
    -> std::optional<typename T::mapped_type> {
  if (idx_map.contains({table_name, attribute_name})) {
    return idx_map.at({table_name, attribute_name});
  }
  return std::nullopt;
}

auto DBEngine::csv_insert(const std::string &table_name,
                          const std::filesystem::path &file) -> query_time_t {

  auto rec_size = m_tables_raw.at(table_name).get_record_size();
  auto pos_getter = [&table_name, this, &rec_size]() {
    // Starting pos
    static std::streampos prev_pos =
        m_tables_raw.at(table_name).next_pos() -
        static_cast<std::streampos>(
            m_tables_raw.at(table_name).get_record_size());
    prev_pos += rec_size;
    return prev_pos;
  };

  std::filesystem::path table_path = TABLES_PATH + table_name;
  std::filesystem::path csv_path = CSV_PATH + file.filename().string() + ".csv";

  if (!std::filesystem::exists(csv_path)) {
    throw std::runtime_error("File does not exist");
  }

  auto &table_heap = m_tables_raw.at(table_name);

  auto [key_type, key_name] = table_heap.get_key_name();
  auto primary_key_idx = table_heap.get_attribute_idx(key_name);

  std::ifstream csv_stream(csv_path);

  // Get csv size
  csv_stream.seekg(0, std::ios::end);
  auto csv_size = csv_stream.tellg();
  auto min_record_count =
      static_cast<ulong>(csv_size / table_heap.get_record_size());
  csv_stream.seekg(0, std::ios::beg);

  std::string current_line;

  // Field types
  auto field_types = table_heap.get_types();
  auto inserted_keys = generate_key_output(field_types, min_record_count);

  std::string key_value = get_sample_value(key_type);

  std::vector<bool> inserted_indexes;

  std::jthread pk_insertion;

  query_time_t times;

  key_cast_and_execute(
      key_type.type,
      [&](ValidIndexType auto sample) {
        // Iterate records
        using pk_type = decltype(sample);

        if (std::is_same_v<int, pk_type>) {
          spdlog::info("inserting with key==int");
        }

        std::vector<std::pair<pk_type, std::streampos>> pk_values;

        if (std::is_same_v<std::vector<std::pair<int, std::streampos>>,
                           decltype(pk_values)>) {
          spdlog::info("inserting into pk_values ==vec int");
        }

        auto pk_init_size =
            SequentialIndex<pk_type>::MIN_BULK_INSERT_SIZE::value /
            sizeof(std::pair<pk_type, HeapFile::pos_type>);
        pk_values.reserve(min_record_count);

        std::vector<Record> records;
        while (std::getline(csv_stream, current_line)) {

          auto fields = current_line | std::ranges::views::split(',');
          records.emplace_back(fields);

          // Iterate fields
          for (ulong curr_field = 0; auto field : fields) {

            insert_field(field, curr_field, field_types, inserted_keys);

            if (curr_field == primary_key_idx) {

              key_value = std::string(field.begin(), field.end());

              switch (key_type.type) {

              case Type::BOOL: {
                spdlog::warn("Bool can't be pk");
                break;
              }

              case Type::INT: {
                std::pair<int, std::streampos> i =
                    std::make_pair(stoi(key_value), pos_getter());
                pk_values.push_back(i);
                break;
              }

              case Type::FLOAT: {
                std::pair<float, std::streampos> i =
                    std::make_pair(stof(key_value), pos_getter());
                pk_values.push_back(i);
                break;
              }
              }

              if (curr_field > pk_init_size) {
                pk_insertion =
                    std::jthread([&inserted_indexes, &table_name, &key_name,
                                  &pk_values, this, &times]() {
                      spdlog::info("Bulk inserting pk - 1");
                      auto bulk_insert_response =
                          m_sequential_indexes.at({table_name, key_name})
                              .bulk_insert<pk_type>(pk_values);
                      inserted_indexes = bulk_insert_response.second;
                      times["PK_BULK_INSERT"] =
                          bulk_insert_response.first.query_time;
                    });
              }
            }

            curr_field++;
          }
        }

        table_heap.bulk_insert(records);

        if (pk_insertion.joinable()) {
          pk_insertion.join();
        }

        spdlog::info("Inserting into primary key index n° - rest {}",
                     pk_values.size());

        for (const auto &elem : pk_values) {
          std::cout << elem.first << ' ' << elem.second << '\n';
        }

        auto rest_pk_insert = m_sequential_indexes.at({table_name, key_name})
                                  .bulk_insert<pk_type>(pk_values);
        auto inserted_bools = rest_pk_insert.second;

        if (!times.contains("PK_BULK_INSERT")) {
          times["PK_BULK_INSERT"] = decltype(times)::mapped_type{};
        }

        times["PK_BULK_INSERT"] += rest_pk_insert.first.query_time;

        spdlog::info("Recieved bool count: {}", inserted_bools.size());

        inserted_indexes.insert(inserted_indexes.end(), inserted_bools.begin(),
                                inserted_bools.end());
      },
      key_value);

  // get non-primary indexes
  std::vector<INDEX_OPTION> available_indexes;
  available_indexes.resize(field_types.size(), INDEX_OPTION::NONE);

  // Fill available_indexes
  auto process_indexes = [&table_name, &available_indexes, &table_heap,
                          &key_name](const auto &indexes, INDEX_OPTION option) {
    auto count = 0;
    for (const auto &[idx, UNUSED] : indexes) {
      if (idx.table == table_name) {

        if (idx.attribute_name != key_name) {
          std::cout << "\tFound available index on :" << idx.attribute_name
                    << ' ' << count << ' ' << option << '\n';
          available_indexes.at(
              table_heap.get_attribute_idx(idx.attribute_name)) += option;
        }
      }
      count++;
    }
  };
  process_indexes(m_sequential_indexes, INDEX_OPTION::SEQUENTIAL);
  process_indexes(m_isam_indexes, INDEX_OPTION::ISAM);
  process_indexes(m_avl_indexes, INDEX_OPTION::AVL);

  // Insert records into available indexes
  auto attribute_names = table_heap.get_attribute_names();

  spdlog::info("Inserting into available indexes");
  for (ulong idx_c = 0; const auto &idx : available_indexes) {

    spdlog::info("Inserting into index n° {}", idx_c);

    switch (field_types.at(idx_c).type) {

    case Type::INT:
      handle_option<int>(
          idx, inserted_keys.at(idx_c), inserted_indexes,
          get_idx(table_name, attribute_names.at(idx_c), m_avl_indexes),
          get_idx(table_name, attribute_names.at(idx_c), m_sequential_indexes),
          get_idx(table_name, attribute_names.at(idx_c), m_isam_indexes),
          pos_getter, times);
      break;
    case Type::FLOAT:
      handle_option<float>(
          idx, inserted_keys.at(idx_c), inserted_indexes,
          get_idx(table_name, attribute_names.at(idx_c), m_avl_indexes),
          get_idx(table_name, attribute_names.at(idx_c), m_sequential_indexes),
          get_idx(table_name, attribute_names.at(idx_c), m_isam_indexes),
          pos_getter, times);
      break;
    case Type::VARCHAR:
    case Type::BOOL:
      spdlog::warn("Bool or varchar can't be indexed. at: inserting into "
                   "available indexes.");
      break;
    }

    idx_c++;
  }
  return times;
}

auto DBEngine::csv_insert(const std::filesystem::path & /*file*/)
    -> query_time_t {
  return {};
}

auto DBEngine::is_table(const std::string &table_name) const -> bool {
  return m_tables_raw.contains(table_name);
}

auto DBEngine::get_table_attributes(const std::string &table_name) const
    -> std::vector<std::string> {
  return m_tables_raw.at(table_name).get_attribute_names();
}

auto DBEngine::get_indexes(const std::string &table_name) const
    -> std::vector<Index_t> {
  return m_index_map.at(table_name);
}
auto DBEngine::get_indexes_names(const std::string &table_name) const
    -> std::vector<std::string> {
  std::vector<std::string> indexes_names;

  for (const auto &avl_index : m_avl_indexes) {
    if (avl_index.first.table == table_name) {
      indexes_names.push_back(avl_index.first.attribute_name);
    }
  }
  for (const auto &isam_index : m_isam_indexes) {
    if (isam_index.first.table == table_name) {
      indexes_names.push_back(isam_index.first.attribute_name);
    }
  }
  for (const auto &sequential_index : m_sequential_indexes) {
    if (sequential_index.first.table == table_name) {
      indexes_names.push_back(sequential_index.first.attribute_name);
    }
  }

  return indexes_names;
}

auto DBEngine::get_comparator(const std::string &table_name, Comp cmp,
                              const std::string &column_name,
                              const std::string &string_to_compare) const
    -> std::function<bool(const Record &record)> {

  auto type = m_tables_raw.at(table_name).get_type(column_name);
  auto index = m_tables_raw.at(table_name).get_attribute_idx(column_name);

  return [type, cmp, string_to_compare, index](const Record &record) -> bool {
    auto attribute_raw = record.m_fields.at(index);
    return cast_and_execute<bool>(
        type.type,
        [&cmp](auto compare_value, auto record_value) -> bool {
          switch (cmp) {
          case EQUAL:
            spdlog::info("EQUAL COMP = {}", record_value);
            if constexpr (std::is_same_v<decltype(compare_value), float>) {
              return compare_value - record_value < FLOAT_EPSILON;
            } else {
              return compare_value == record_value;
            }
          case LE:
            spdlog::info("EQUAL LE = {}", record_value);
            return compare_value <= record_value;
          case G:
            return compare_value > record_value;

          case L:
            return compare_value < record_value;
          case GE:
            return compare_value >= record_value;
          }
          throw std::runtime_error("Not valid comparator");
        },
        string_to_compare, attribute_raw);
    return cmp == EQUAL;
  };
}

auto DBEngine::sort_attributes(const std::string &table_name,
                               const std::vector<std::string> &attributes) const
    -> std::vector<std::string> {
  // sort based on m_tables_raw.get_attribute_idx(string) value

  std::vector<std::string> sorted_attributes = attributes;
  std::ranges::sort(sorted_attributes, [&](const auto &val1, const auto &val2) {
    return m_tables_raw.at(table_name).get_attribute_idx(val1) <
           m_tables_raw.at(table_name).get_attribute_idx(val2);
  });

  return sorted_attributes;
}

void DBEngine::clean_table(const std::string &table_name) {
  std::filesystem::remove_all(TABLES_PATH + table_name);
}

void DBEngine::drop_table(const std::string &table_name) {

  spdlog::warn("Droping table: {}", table_name);

  // Remove heap file
  std::filesystem::remove_all(TABLES_PATH + table_name);

  // Remove asociated indexes
  std::filesystem::remove_all(INDEX_PATH "Sequential/" + table_name);
  std::filesystem::remove_all(INDEX_PATH "AVL/" + table_name);
  std::filesystem::remove_all(INDEX_PATH "ISAM/" + table_name);

  m_tables_raw.erase(table_name);

  for (auto &idx : m_avl_indexes) {
    if (idx.first.table == table_name) {
      m_avl_indexes.erase(idx.first);
    }
  }
  for (auto &idx : m_isam_indexes) {
    if (idx.first.table == table_name) {
      m_avl_indexes.erase(idx.first);
    }
  }
  for (auto &idx : m_sequential_indexes) {
    if (idx.first.table == table_name) {
      m_avl_indexes.erase(idx.first);
    }
  }
}

void DBEngine::generate_directories() {

  std::string base_path(BASE_PATH);

  bool created_base_dir = create_directory(base_path);
  spdlog::info(created_base_dir ? "Initialized filesystem"
                                : "Loaded filesystem");

  // Tables metadata
  if (create_directory(TABLES_PATH)) {
    spdlog::info("Created Tables subdir");
  }

  // Metadata
  if (create_directory(METADATA_PATH)) {
    spdlog::info("Created Metadata subdir");
  }

  // Indexes
  if (create_directory(INDEX_PATH)) {
    spdlog::info("Created Indexes subdir");
  }

  // Create subdirs for each index type
  for (const auto &idx_name : Constants::INDEX_TYPES) {

    if (create_directory(INDEX_PATH "/" + idx_name)) {
      spdlog::info("Created {} subdir", idx_name);
    }
  }

  spdlog ::info("DBEngine created");
}
