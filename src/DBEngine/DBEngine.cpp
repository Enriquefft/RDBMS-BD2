#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

#include <spdlog/spdlog.h>

#include "Constants.hpp"
#include "DBEngine.hpp"

using std::filesystem::create_directory;
using std::filesystem::exists;

DBEngine::DBEngine() {
  generate_directories();

  // Read tables raw data

  // Load Indexes
}

auto DBEngine::create_table(std::string table_name,
                            const std::string &primary_key,
                            std::vector<Type> types,
                            std::vector<std::string> attribute_names) -> bool {

  // Check if table already exists
  std::string table_path = TABLES_PATH + table_name;

  //
  if (exists(table_path)) {
    spdlog::warn("Table {} already exists doing nothing.", table_name);
    return false;
  }

  //

  //
  create_directory(table_path);

  HeapFile heap_file(table_path, std::move(types), std::move(attribute_names),
                     primary_key);

  m_tables_raw.insert({table_name, heap_file});

  SequentialIndex sequential_index;

  m_sequential_indexes.insert({{table_name, primary_key}, sequential_index});
  return true;
}

auto DBEngine::get_table_names() -> std::vector<std::string> {

  std::vector<std::string> table_names;

  for (const auto &table : m_tables_raw) {
    table_names.push_back(table.first);
  }
  return table_names;
}

auto DBEngine::stob(std::string str) -> bool {

  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  static std::unordered_set<std::string> valid_strings = {
      "YES", "Y", "SI", "S", "V", "VERDADERO", "T", "TRUE"};
  return valid_strings.contains(str);
}

template <typename Func>
void DBEngine::cast_and_execute(Type::types type,
                                const std::string &attribute_value, Func func) {
  switch (type) {
  case Type::types::INT: {
    break;
  }
  case Type::types::FLOAT: {
    float key_value = std::stof(attribute_value);
    func(key_value);
    break;
  }
  case Type::types::BOOL: {
    bool key_value = stob(attribute_value);
    func(key_value);
    break;
  }
  case Type::types::VARCHAR: {
    func(attribute_value);
    break;
  }
  }
}

template <typename Func>
void DBEngine::cast_and_execute(Type::types type, const std::string &att1,
                                const std::string &att2, Func func) {
  switch (type) {
  case Type::types::INT: {
    int value_1 = std::stoi(att1);
    int value_2 = std::stoi(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::FLOAT: {
    float value_1 = std::stof(att1);
    float value_2 = std::stof(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::BOOL: {
    bool value_1 = stob(att1);
    bool value_2 = stob(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::VARCHAR: {
    func(att1, att2);
    break;
  }
  }
}

auto DBEngine::search(const std::string &table_name, const Attribute &key,
                      const std::function<bool(Record)> &expr,
                      const std::vector<std::string> &selected_attributes)
    -> std::string {

  HeapFile::pos_type pos = 0;

  //

  //
  auto key_type = m_tables_raw.at(table_name).get_type(key);

  cast_and_execute(key_type.type, key.value,
                   [this, &key, &pos](auto key_value) {
                     for (const auto &idx : m_sequential_indexes) {
                       if (idx.second.get_attribute_name() == key.name) {
                         pos = idx.second.search(key_value);
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
    -> std::vector<std::string> {

  if (begin_key.name != end_key.name) {
    throw std::runtime_error("Cant apply range_search to different attributes");
  }

  std::vector<HeapFile::pos_type> positions;

  auto key_type = m_tables_raw.at(table_name).get_type(begin_key);

  cast_and_execute(
      key_type.type, begin_key.value, end_key.value,
      [this, &positions, &begin_key](auto begin_key_value, auto end_key_value) {
        for (const auto &idx : m_sequential_indexes) {
          if (idx.second.get_attribute_name() == begin_key.name) {
            positions = idx.second.range_search(begin_key_value, end_key_value);
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

  auto inserted_pos = m_tables_raw.at(table_name).add(value);

  bool inserted = false;

  auto [type, key] = m_tables_raw.at(table_name).get_key(value);

  for (const auto &idx : m_sequential_indexes) {
    if (idx.first.table == table_name && idx.first.attribute_name == key.name) {
      cast_and_execute(type.type, key.value,
                       [&idx, &inserted_pos, &inserted](auto key_val) {
                         inserted = idx.second.add(key_val, inserted_pos);
                       });
    }
  }
  return inserted;
}

auto DBEngine::remove(const std::string &table_name, const Attribute &key)
    -> bool {

  HeapFile::pos_type raw_pos;
  for (const auto &idx : m_sequential_indexes) {
    if (idx.first.table == table_name) {
      auto type = m_tables_raw.at(table_name).get_type(key);
      cast_and_execute(type.type, key.value, [&raw_pos, &idx](auto key_value) {
        raw_pos = idx.second.remove(key_value);
      });
    }
  }
  if (raw_pos == -1) {
    return false;
  }

  m_tables_raw.at(table_name).remove(raw_pos);
  return true;
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
