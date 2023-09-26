#include "HeapFile.hpp"
#include "Constants.hpp"
#include "Record/Record.hpp"
#include "Utils/File.hpp"

#include <bits/chrono.h>
#include <chrono>
#include <iostream>
#include <numeric>
#include <response.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int DEFAULT_DELETED = -1;

using DB_ENGINE::HeapFile;

HeapFile::HeapFile(std::string table_name, std::vector<Type> types,
                   std::vector<std::string> attribute_names,
                   std::string primary_key)
    : C_RECORD_SIZE(std::accumulate(
          types.begin(), types.end(), static_cast<uint8_t>(0),
          [](uint8_t sum, const Type &type) { return sum + type.size; })),
      m_table_name(std::move(table_name)),
      m_table_path(TABLES_PATH + m_table_name + "/"),
      m_metadata(std::move(attribute_names), std::move(types),
                 std::move(primary_key)) {

  open_or_create(m_table_path + DATA_FILE);

  if (!read_metadata()) {
    write_metadata();
  }
}

HeapFile::HeapFile(const std::filesystem::path &table_path)
    : m_table_name(table_path.filename()),
      m_table_path(table_path.string() + "/") {
  read_metadata();
}

HeapFile::TableMetadata::TableMetadata(
    std::vector<std::string> _attribute_names,
    std::vector<Type> _attribute_types, std::string _primary_key)
    : attribute_names(std::move(_attribute_names)),
      attribute_types(std::move(_attribute_types)),
      primary_key(std::move(_primary_key)) {}

auto HeapFile::load()
    -> std::pair<std::vector<Record>, std::chrono::milliseconds> {

  spdlog::info("Loading all data");

  m_file_stream.open(m_table_path + DATA_FILE, std::ios::binary | std::ios::in);

  if (std::filesystem::exists(m_table_path + DATA_FILE)) {
  }

  if (!m_file_stream) {
    spdlog::error("Could not open file {}", m_table_path + DATA_FILE);
  }

  std::vector<Record> records(m_metadata.record_count);
  spdlog::info("Found {} records", m_metadata.record_count);

  auto start_time = std::chrono::high_resolution_clock::now();
  for (auto &record : records) {
    record.read(m_file_stream, m_metadata.attribute_types);
  }
  auto end_time = std::chrono::high_resolution_clock::now();

  m_file_stream.close();
  return {records, std::chrono::duration_cast<std::chrono::milliseconds>(
                       end_time - start_time)};
}

auto HeapFile::add(const Record &record) -> pos_type {

  m_file_stream.open(m_table_path + DATA_FILE,
                     std::ios::binary | std::ios::out | std::ios::app);
  auto initial_pos = next_pos();

  spdlog::info("Adding record at {}", static_cast<std::streamoff>(initial_pos));

  record.write(m_file_stream, m_metadata.attribute_types);
  m_file_stream.close();

  m_metadata.record_count++;
  write_metadata();

  return initial_pos;
}

auto HeapFile::bulk_insert(const std::vector<Record> &records)
    -> ::Index::Response {

  m_file_stream.open(m_table_path + DATA_FILE,
                     std::ios::binary | std::ios::out);
  auto initial_pos = next_pos();
  m_file_stream.seekp(initial_pos, std::ios::beg);

  std::vector<pos_type> positions(records.size());

  auto start_time = std::chrono::high_resolution_clock::now();

  for (ulong idx = 0; const auto &rec : records) {
    rec.write(m_file_stream, m_metadata.attribute_types);
    positions.at(idx) =
        initial_pos + static_cast<std::streamoff>(idx) * get_record_size();
    idx++;
  }
  auto end_time = std::chrono::high_resolution_clock::now();

  m_file_stream.close();

  m_metadata.record_count += records.size();
  write_metadata();

  return {std::move(positions),
          std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                start_time)};
}

auto HeapFile::next_pos() const -> pos_type {

  std::ifstream stream(m_table_path + DATA_FILE,
                       std::ios::binary | std::ios::ate);

  auto pos = static_cast<pos_type>(stream.tellg());
  return pos;
}

auto HeapFile::read(const pos_type &pos) -> std::pair<Record, time_t> {

  auto start_time = std::chrono::high_resolution_clock::now();

  m_file_stream.open(m_table_path + DATA_FILE, std::ios::binary | std::ios::in);
  m_file_stream.seekg(pos, std::ios::beg);

  Record record;

  record.read(m_file_stream, m_metadata.attribute_types);
  m_file_stream.close();

  auto end_time = std::chrono::high_resolution_clock::now();

  return {record, std::chrono::duration_cast<time_t>(end_time - start_time)};
}

auto HeapFile::read(const std::vector<pos_type> &positions)
    -> std::pair<std::vector<Record>, time_t> {

  std::vector<Record> response(positions.size());
  auto start_time = std::chrono::high_resolution_clock::now();

  m_file_stream.open(m_table_path + DATA_FILE, std::ios::binary | std::ios::in);

  for (ulong idx = 0; const auto &pos : positions) {
    m_file_stream.seekg(pos, std::ios::beg);
    response.at(idx).read(m_file_stream, m_metadata.attribute_types);
    idx++;
  }

  m_file_stream.close();

  auto end_time = std::chrono::high_resolution_clock::now();

  return {response, std::chrono::duration_cast<time_t>(end_time - start_time)};
}

auto HeapFile::remove(const pos_type &pos) -> std::pair<bool, time_t> {

  auto start_time = std::chrono::high_resolution_clock::now();

  m_file_stream.open(m_table_path + DATA_FILE,
                     std::ios::binary | std::ios::out);
  m_file_stream.seekg(pos, std::ios::beg);

  Record record;
  auto &succes = record.read(m_file_stream, m_metadata.attribute_types);
  record.status = DELETED;

  m_file_stream.seekg(pos, std::ios::beg);
  record.write(m_file_stream, m_metadata.attribute_types);

  m_file_stream.close();

  auto end_time = std::chrono::high_resolution_clock::now();

  return {succes.good(),
          std::chrono::duration_cast<time_t>(end_time - start_time)};
}

void HeapFile::update_first_deleted(pos_type pos) {
  m_metadata.first_deleted = pos;
  write_metadata();
}

auto HeapFile::filter(Record &record,
                      const std::vector<std::string> &selected_attributes,
                      const query_time_t &times) const -> QueryResponse {

  // Get indexes of attributes to filter
  std::vector<uint8_t> attribute_idxs;

  attribute_idxs.reserve(selected_attributes.size());
  for (const auto &attr_name : selected_attributes) {
    attribute_idxs.push_back(m_metadata.get_attribute_idx(attr_name));
  }

  // Iterate over each word in reverse order to prevent iterator invalidation
  for (int i = static_cast<int>(record.m_fields.size()) - 1; i >= 0; --i) {
    // If the index is not in the indices vector, remove the word
    if (!std::binary_search(attribute_idxs.begin(), attribute_idxs.end(), i)) {
      record.m_fields.erase(record.begin() + i);
    }
  }
  return {{record}, times};
}

auto HeapFile::filter(std::vector<Record> &records,
                      const std::vector<std::string> &selected_attributes,
                      const query_time_t &times) const -> QueryResponse {

  std::vector<uint8_t> attribute_idxs;
  attribute_idxs.reserve(selected_attributes.size());
  for (const auto &attr_name : selected_attributes) {
    attribute_idxs.push_back(m_metadata.get_attribute_idx(attr_name));
  }

  // We assume the indexes are already sorted

  // Iterate over each record
  for (auto &record : records) {
    // Iterate over each word in reverse order
    for (int i = static_cast<int>(record.m_fields.size()) - 1; i >= 0; --i) {
      // If the index is not in the indices vector, remove the word
      if (!std::binary_search(attribute_idxs.begin(), attribute_idxs.end(),
                              i)) {
        record.m_fields.erase(record.begin() + i);
      }
    }
  }
  return {records, times};
}

void HeapFile::write_metadata() {
  std::ofstream metadata(m_table_path + METADATA_FILE, std::ios::binary);
  if (!metadata.write(reinterpret_cast<const char *>(&m_metadata),
                      sizeof(m_metadata))) {
    spdlog::error("Could not write metadata of table {}", m_table_name);
  }
  spdlog::info("Metadata of table {} updated\n", m_table_name);
  metadata.close();
}

auto HeapFile::TableMetadata::get_attribute_idx(
    const std::string &attribute_name) const -> uint8_t {

  uint8_t counter = 0;
  for (const auto &attribute : attribute_names) {
    if (attribute == attribute_name) {
      return counter;
    }
    counter++;
  }
  throw std::runtime_error("Attribute not found");
}

auto HeapFile::string_cast(const Type &type, const char *data) -> std::string {

  switch (type.type) {
  case Type::BOOL: {
    bool value = *reinterpret_cast<const bool *>(data);
    return value ? "true" : "false";
  }

  case Type::INT: {
    int int_value = *reinterpret_cast<const int *>(data);
    return std::to_string(int_value);
  }
  case Type::FLOAT: {
    float float_value = *reinterpret_cast<const float *>(data);
    return std::to_string(float_value);
  }
  case Type::VARCHAR: {
    std::string value(data, type.size);
    return value;
  }
  }
  throw std::runtime_error("Invalid type");
}

auto HeapFile::rec_to_string(const Record &rec) -> std::string {

  std::string record;

  for (const auto &m_field : rec.m_fields) {

    record += {m_field.begin(), m_field.end()};
  }
  return record;
}

auto HeapFile::get_record_size() const -> uint8_t { return C_RECORD_SIZE; }

auto HeapFile::get_type(const std::string &attribute_name) const -> Type {
  auto attribute_idx = m_metadata.get_attribute_idx(attribute_name);
  return m_metadata.attribute_types.at(attribute_idx);
}

auto HeapFile::get_type(const Attribute &attribute) const -> Type {
  return get_type(attribute.name);
}
auto HeapFile::get_types() const -> std::vector<Type> {
  return m_metadata.attribute_types;
}

auto HeapFile::get_key(const Record &record) const
    -> std::pair<Type, Attribute> {

  auto key_idx = m_metadata.get_attribute_idx(m_metadata.primary_key);
  auto key_type = m_metadata.attribute_types.at(key_idx);

  Attribute key_attribute = {m_metadata.attribute_names.at(key_idx),
                             record.m_fields.at(key_idx)};

  return {key_type, key_attribute};
}

auto HeapFile::get_key_name() const -> std::pair<Type, std::string> {

  auto key_idx = m_metadata.get_attribute_idx(m_metadata.primary_key);
  auto key_type = m_metadata.attribute_types.at(key_idx);
  return {key_type, m_metadata.attribute_names.at(key_idx)};
}
auto HeapFile::get_key_idx() const -> uint8_t {
  return m_metadata.get_attribute_idx(m_metadata.primary_key);
}

auto HeapFile::get_attribute_names() const -> std::vector<std::string> {
  return m_metadata.attribute_names;
}
auto HeapFile::read_metadata() -> bool {

  open_or_create(m_table_path + METADATA_FILE);

  std::ifstream file(
      m_table_path + METADATA_FILE,
      std::ios::binary |
          std::ios::ate); // Already has been checked that it exists

  // File must contain a single integer
  std::streamsize size = file.tellg();

  if (size != sizeof(TableMetadata)) {
    spdlog::warn("Metadata file {} is corrupted or empty",
                 m_table_path + METADATA_FILE);
    file.close();
    return false;
  }

  file.seekg(0, std::ios::beg);

  // The file must be readable
  if (!file.read(reinterpret_cast<char *>(&m_metadata), sizeof(int))) {
    spdlog::error("Error reading metadata from file {}",
                  m_table_path + METADATA_FILE);
    return false;
  }

  return true;
}
