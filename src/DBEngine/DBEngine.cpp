#include <algorithm>
#include <cctype>
#include <filesystem>
#include <limits>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

#include <spdlog/spdlog.h>

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
  case Type::BOOL:
    SequentialIndex<bool> sequential_index_bool;
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_bool)});
    break;
  case Type::INT:
    SequentialIndex<int> sequential_index_int;
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_int)});
    break;
  case Type::FLOAT:
    SequentialIndex<float> sequential_index_float;
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_float)});
    break;
  case Type::VARCHAR:
    SequentialIndex<std::string> sequential_index_str;
    m_sequential_indexes.insert(
        {{table_name, primary_key},
         SequentialIndexContainer(sequential_index_str)});
    break;
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
    -> std::string {

  HeapFile::pos_type pos = 0;

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

void DBEngine::drop_table(const std::string &table_name) {
  std::filesystem::remove_all(TABLES_PATH + table_name);
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
