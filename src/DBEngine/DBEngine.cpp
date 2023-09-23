#include <algorithm>
#include <bitset>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
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

using DB_ENGINE::DBEngine;

using ::std::filesystem::create_directory;
using ::std::filesystem::exists;

constexpr float FLOAT_EPSILON = 0.001F;

DB_ENGINE::DBEngine::DBEngine() {
  generate_directories();

  // Read tables raw data

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
    SequentialIndex<std::string> sequential_index_str(table_name, primary_key,
                                                      true);
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_str)});
    break;
  }
  case Type::BOOL: {
    spdlog::error("Bool can't be indexed. at: table_creation");
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

  HeapFile::pos_type pos = 0;

  response_time time;

  auto key_type = m_tables_raw.at(table_name).get_type(key);

  key_cast_and_execute(
      key_type.type, key.value, [this, &key, &pos, &time](auto key_value) {
        for (const auto &idx : m_sequential_indexes) {
          if (idx.second.get_attribute_name() == key.name) {

            auto search_response = idx.second.search(key_value);
            pos = search_response.first;
            time = search_response.second;
            break;
          }
        }
      });

  Record record = m_tables_raw.at(table_name).read(pos);

  if (!expr(record)) {
    return {};
  }

  return m_tables_raw.at(table_name).filter(record, selected_attributes);
}
auto DBEngine::range_search(const std::string &table_name,
                            const Attribute &begin_key,
                            const Attribute &end_key,
                            const std::function<bool(Record)> &expr,
                            const std::vector<std::string> &selected_attributes)
    -> QueryResponse {

  if (begin_key.name != end_key.name) {
    throw std::runtime_error("Cant apply range_search to different attributes");
  }

  std::vector<HeapFile::pos_type> positions;
  response_time time;

  auto key_type = m_tables_raw.at(table_name).get_type(begin_key);

  key_cast_and_execute(
      key_type.type, begin_key.value, end_key.value,
      [this, &positions, &begin_key, &time](auto begin_key_value,
                                            auto end_key_value) {
        for (const auto &idx : m_sequential_indexes) {
          if (idx.second.get_attribute_name() == begin_key.name) {
            auto idx_response =
                idx.second.range_search(begin_key_value, end_key_value);
            positions = idx_response.records;
            time = idx_response.query_time;
          }
        }
      });

  std::vector<Record> response;

  for (const auto &pos : positions) {
    auto rec = m_tables_raw.at(table_name).read(pos);

    if (expr(rec)) {
      response.push_back(rec);
    }
  }
  return m_tables_raw.at(table_name).filter(response, selected_attributes);
}

auto DBEngine::add(const std::string &table_name, const Record &value) -> bool {

  bool inserted = false;

  auto [type, key] = m_tables_raw.at(table_name).get_key(value);

  auto inserted_pos = m_tables_raw.at(table_name).next_pos();
  response_time time;

  for (auto &idx : m_sequential_indexes) {
    if (idx.first.table == table_name && idx.first.attribute_name == key.name) {
      key_cast_and_execute(
          type.type, key.value,
          [&idx, &inserted_pos, &inserted, &time](auto key_val) {
            auto add_response = idx.second.add(key_val, inserted_pos);
            inserted = add_response.first;
            time = add_response.second;
          });
    }
  }

  if (inserted) {
    m_tables_raw.at(table_name).add(value);
  }

  return inserted;
}

auto DBEngine::remove(const std::string &table_name, const Attribute &key)
    -> bool {

  response_time time;
  HeapFile::pos_type raw_pos;
  for (const auto &idx : m_sequential_indexes) {
    if (idx.first.table == table_name) {
      auto type = m_tables_raw.at(table_name).get_type(key);
      key_cast_and_execute(
          type.type, key.value, [&raw_pos, &idx, &time](auto key_value) {
            auto remove_response = idx.second.remove(key_value);
            raw_pos = remove_response.first;
            time = remove_response.second;
          });
    }
  }
  if (raw_pos == -1) {
    return false;
  }

  m_tables_raw.at(table_name).remove(raw_pos);
  return true;
}

enum class INDEX_OPTION : uint8_t {
  NONE = 0U,
  AVL = 1U << 0U,
  ISAM = 1U << 1U,
  SEQUENTIAL = 1U << 2U
};

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
    spdlog::info("creating IndexInsert with {}",
                 typeid(decltype(_values)).name());
    if (std::is_same_v<std::vector<int>, decltype(_values)>) {
      spdlog::info("creating IndexInsert with int");
    }
  }

  template <ValidType T, class... Args> void emplace_back(Args &&...args) {
    if (std::holds_alternative<std::vector<T>>(values)) {
      std::get<std::vector<T>>(values).emplace_back(
          std::forward<Args>(args)...);
    } else {
      spdlog::error("vector doesn't holds expected type");
    }
  }
  template <ValidType T> void reserve(const auto &reserve_size) {
    if (std::holds_alternative<std::vector<T>>(values)) {
      std::get<std::vector<T>>(values).reserve(reserve_size);
    } else {
      spdlog::error("vector doesn't holds expected type in reserve()");
    }
  }
  template <ValidType T> [[nodiscard]] T at(const auto &idx) const {
    if (std::holds_alternative<std::vector<T>>(values)) {
      return std::get<std::vector<T>>(values).at(idx);
    }
    spdlog::error("vector doesn't holds expected type in 'at'");
    throw std::runtime_error("hold_wrong");
  }
  template <ValidType T> [[nodiscard]] auto size() const -> ulong {
    if (std::holds_alternative<std::vector<T>>(values)) {
      return std::get<std::vector<T>>(values).size();
    }
    spdlog::error("vector doesn't holds expected type in size()");
    throw std::runtime_error("hold_wrong");
  }

  std::variant<std::vector<int>, std::vector<float>, std::vector<std::string>>
      values;
};

template <ValidType T>
static void handleOption(const INDEX_OPTION &option,
                         const IndexInsert &inserted_keys,
                         const std::vector<bool> &pk_insertion, auto &avl,
                         auto &sequential, auto &isam, const auto &get_pos) {

  std::bitset<3> options(static_cast<uint8_t>(option));

  std::vector<std::pair<T, std::streampos>> inserted_indexes;
  inserted_indexes.reserve(inserted_keys.size<T>());

  for (ulong i = 0; i < inserted_keys.size<T>(); i++) {
    if (pk_insertion.at(i)) {
      inserted_indexes.emplace_back(
          std::make_pair(inserted_keys.at<T>(i), get_pos()));
    }
  }

  if (options.test(static_cast<size_t>(INDEX_OPTION::AVL))) {
    std::jthread avl_insert([&avl, &inserted_indexes]() {
      avl.template bulk_insert<T>(inserted_indexes);
    });
  }
  if (options.test(static_cast<size_t>(INDEX_OPTION::ISAM))) {
    std::jthread isam_insert([&isam, &inserted_indexes]() {
      isam.template bulk_insert<T>(inserted_indexes);
    });
  }
  if (options.test(static_cast<size_t>(INDEX_OPTION::SEQUENTIAL))) {
    std::jthread sequential_insert([&sequential, &inserted_indexes]() {
      sequential.bulk_insert(inserted_indexes);
    });
  }
}

static auto generate_key_output(const std::vector<DB_ENGINE::Type> &field_types,
                                const ulong &min_record_count)
    -> std::vector<IndexInsert> {

  std::vector<IndexInsert> inserted_keys;
  inserted_keys.resize(field_types.size());
  spdlog::info("Reserving vector of idxInsert");

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
    case DB_ENGINE::Type::VARCHAR: {
      IndexInsert insert_vec((std::vector<std::string>()));
      inserted_keys.at(idx) = insert_vec;
      inserted_keys.at(idx).reserve<std::string>(min_record_count);
      break;
    }
    case DB_ENGINE::Type::BOOL:
      continue;
    }
  }
  return inserted_keys;
}

void insert_field(const auto &field, const ulong &curr_field,
                  const std::vector<DB_ENGINE::Type> &field_types,
                  std::vector<IndexInsert> &inserted_keys) {

  switch (field_types.at(curr_field).type) {
  case DB_ENGINE::Type::INT: {
    auto field_val = std::string(field.begin(), field.end());
    spdlog::info("converting {} to int", field_val);
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
  case DB_ENGINE::Type::VARCHAR: {
    inserted_keys.at(curr_field)
        .template emplace_back<std::string>(field.begin(), field.end());
    break;
  }
  case DB_ENGINE::Type::BOOL:
    spdlog::error("Bool can't be indexed at field insertion");
    break;
  }
}
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
}

void DBEngine::csv_insert(const std::string &table_name,
                          const std::filesystem::path &file) {

  auto pos_getter = [&table_name, this]() {
    // Starting pos
    static std::streampos prev_pos =
        m_tables_raw.at(table_name).next_pos() -
        static_cast<std::streampos>(
            m_tables_raw.at(table_name).get_record_size());
    prev_pos += m_tables_raw.at(table_name).get_record_size();
    return prev_pos;
  };

  std::filesystem::path table_path = TABLES_PATH + table_name;
  std::filesystem::path csv_path = CSV_PATH + file.filename().string() + ".csv";

  if (!std::filesystem::exists(csv_path)) {
    throw std::runtime_error("File does not exist");
  }

  auto &table_heap = m_tables_raw.at(table_name);

  spdlog::info("Engine csv_insert: {}, {}", table_name, csv_path.string());

  auto [key_type, key_name] = table_heap.get_key_name();
  auto key_idx = table_heap.get_attribute_idx(key_name);

  std::ifstream csv_stream(csv_path);
  spdlog::info("Started stream");

  spdlog::info("key_type: {}", key_type.to_string());
  spdlog::info("key_name: {}", key_name);

  // Get csv size
  csv_stream.seekg(0, std::ios::end);
  auto csv_size = csv_stream.tellg();
  auto min_record_count =
      static_cast<ulong>(csv_size / table_heap.get_record_size());
  csv_stream.seekg(0, std::ios::beg);
  spdlog::info("csv_size: {}", static_cast<ulong>(csv_size));

  std::string current_line;

  // Field types
  auto field_types = table_heap.get_types();
  auto inserted_keys = generate_key_output(field_types, min_record_count);
  spdlog::info("Generated key outputs");

  std::string key_value = get_sample_value(key_type);

  std::vector<bool> inserted_indexes;

  std::jthread pk_insertion;

  spdlog::info("Started iterating csv file");
  key_cast_and_execute(key_type.type, key_value, [&](auto pk_value) {
    // Iterate records
    using pk_type = decltype(pk_value);

    std::vector<std::pair<pk_type, std::streampos>> pk_values;

    auto pk_init_size = SequentialIndex<pk_type>::MIN_BULK_INSERT_SIZE::value /
                        sizeof(std::pair<pk_type, HeapFile::pos_type>);
    pk_values.reserve(min_record_count);

    std::vector<Record> records;
    spdlog::info("Iterating csv file");
    while (std::getline(csv_stream, current_line)) {
      spdlog::info("Iterating line: {}", current_line);

      auto fields = current_line | std::ranges::views::split(',');
      records.emplace_back(fields);

      // Iterate fields
      for (ulong curr_field = 0; auto field : fields) {

        insert_field(field, curr_field, field_types, inserted_keys);

        if (curr_field % key_idx == 0) {
          key_value = std::string(field.begin(), field.end());
          pk_values.emplace_back(std::make_pair(pk_value, pos_getter()));

          if (curr_field > pk_init_size) {
            pk_insertion = std::jthread([&inserted_indexes, &table_name,
                                         &key_name, &pk_values, this]() {
              auto bulk_insert_response =
                  m_sequential_indexes.at({table_name, key_name})
                      .bulk_insert<pk_type>(pk_values);
              inserted_indexes = bulk_insert_response.second;
            });
          }
        }

        curr_field++;
      }
    }

    table_heap.bulk_insert(records);

    pk_insertion.join();
    auto inserted_bools = m_sequential_indexes.at({table_name, key_name})
                              .bulk_insert<pk_type>(pk_values)
                              .second;
    inserted_indexes.insert(inserted_indexes.end(), inserted_bools.begin(),
                            inserted_bools.end());
  });

  // get non-primary indexes
  std::vector<INDEX_OPTION> available_indexes;

  // Fill available_indexes
  auto process_indexes = [&table_name, &available_indexes, &table_heap](
                             const auto &indexes, INDEX_OPTION option) {
    for (const auto &[idx, UNUSED] : indexes) {
      if (idx.table == table_name) {
        available_indexes.at(
            table_heap.get_attribute_idx(idx.attribute_name)) += option;
      }
    }
  };
  process_indexes(m_sequential_indexes, INDEX_OPTION::SEQUENTIAL);
  process_indexes(m_isam_indexes, INDEX_OPTION::ISAM);
  process_indexes(m_avl_indexes, INDEX_OPTION::AVL);

  // Insert records into available indexes
  auto attribute_names = m_tables_raw.at(table_name).get_attribute_names();

  for (ulong idx_c = 0; const auto &idx : available_indexes) {

    switch (field_types.at(idx_c).type) {

    case Type::INT:
      handleOption<int>(
          idx, inserted_keys.at(idx_c), inserted_indexes,
          m_avl_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_sequential_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_isam_indexes.at({table_name, attribute_names.at(idx_c)}),
          pos_getter);
      break;
    case Type::FLOAT:
      handleOption<float>(
          idx, inserted_keys.at(idx_c), inserted_indexes,
          m_avl_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_sequential_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_isam_indexes.at({table_name, attribute_names.at(idx_c)}),
          pos_getter);
      break;
    case Type::VARCHAR:
      handleOption<std::string>(
          idx, inserted_keys.at(idx_c), inserted_indexes,
          m_avl_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_sequential_indexes.at({table_name, attribute_names.at(idx_c)}),
          m_isam_indexes.at({table_name, attribute_names.at(idx_c)}),
          pos_getter);
      break;
    case Type::BOOL:
      spdlog::error(
          "Bool can't be indexed. at: inserting into available indexes.");
      break;
    }

    idx_c++;
  }
}

auto DBEngine::csv_insert(const std::filesystem::path & /*file*/) -> bool {
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

  return [&type, &cmp, &string_to_compare, &index](const Record &record) {
    const auto *attribute_raw = record.m_fields.at(index).data();
    cast_and_execute(
        type.type, string_to_compare, attribute_raw,
        [&cmp](auto compare_value, auto record_value) {
          switch (cmp) {
          case EQUAL:
            if constexpr (std::is_same_v<decltype(compare_value), float>) {
              return compare_value - record_value < FLOAT_EPSILON;
            } else {
              return compare_value == record_value;
            }
          case LE:
            return compare_value <= record_value;
          case G:
            return compare_value > record_value;

          case L:
            return compare_value < record_value;
          case GE:
            return compare_value >= record_value;
          }
          throw std::runtime_error("Not valid comparator");
        });
    return cmp == EQUAL;
  };
}

void DBEngine::clean_table(const std::string &table_name) {
  std::filesystem::remove_all(TABLES_PATH + table_name);
}
void DBEngine::drop_table(const std::string &table_name) {
  std::filesystem::remove_all(TABLES_PATH + table_name);
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
