#include "HeapFile.hpp"
#include "Constants.hpp"
#include "Utils/File.hpp"

#include <numeric>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int DEFAULT_DELETED = -1;

HeapFile::HeapFile(std::string table_name, std::vector<Type> types,
                   std::vector<std::string> attribute_names,
                   std::string primary_key)
    : C_RECORD_SIZE(std::accumulate(
          types.begin(), types.end(), static_cast<uint8_t>(0),
          [](uint8_t sum, const Type &type) { return sum + type.size; })),
      m_table_name(std::move(table_name)),
      m_table_path(TABLES_PATH "/" + m_table_name + "/"),
      m_metadata(std::move(attribute_names), std::move(types),
                 std::move(primary_key)) {

  open_or_create(m_table_path + DATA_FILE);

  if (!read_metadata()) {
    update_first_deleted(DEFAULT_DELETED);
  }
}

void HeapFile::update_first_deleted(pos_type pos) {
  m_metadata.first_deleted = pos;
  std::ofstream metadata(m_table_path + METADATA_FILE, std::ios::binary);
  metadata.write(reinterpret_cast<const char *>(&m_metadata.first_deleted),
                 sizeof(m_metadata.first_deleted));
  metadata.close();
}

auto HeapFile::TableMetadata::get_attribute_idx(
    const std::string &attribute_name) const -> uint8_t {

  auto counter = 0;
  for (const auto &attribute : attribute_names) {
    if (attribute == attribute_name) {
      return static_cast<uint8_t>(counter);
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

auto HeapFile::get_type(const Attribute &attribute) -> Type {

  auto attribute_idx = m_metadata.get_attribute_idx(attribute.name);
  return m_metadata.attribute_types.at(attribute_idx);
}

auto HeapFile::get_key(const Record &record) -> std::pair<Type, Attribute> {

  auto key_idx = m_metadata.get_attribute_idx(m_metadata.primary_key);

  auto key_type = m_metadata.attribute_types.at(key_idx);

  std::string key_value = string_cast(key_type, record.fields.at(key_idx));

  Attribute key_attribute = {m_metadata.attribute_names.at(key_idx), key_value};

  return {key_type, key_attribute};
}

auto HeapFile::read(const pos_type &pos) -> Record {
  m_file_stream.open(m_table_path + DATA_FILE, std::ios::binary | std::ios::in);

  m_file_stream.seekg(pos, std::ios::beg);

  Record record;

  record.read(m_file_stream, m_metadata.attribute_types);

  return record;
}

auto HeapFile::read_metadata() -> bool {

  open_or_create(m_table_path + METADATA_FILE);

  std::ifstream file(
      m_table_path + METADATA_FILE,
      std::ios::binary |
          std::ios::ate); // Already has been checked that it exists

  // File must contain a single integer
  std::streamsize size = file.tellg();
  if (size != sizeof(int)) {
    spdlog::error("Metadata file {} is corrupted.",
                  m_table_path + METADATA_FILE);
    file.close();
    return false;
  }

  file.seekg(0, std::ios::beg);

  int first_deleted = 0;
  // The file must be readable
  if (!file.read(reinterpret_cast<char *>(&first_deleted), sizeof(int))) {
    spdlog::error("Error reading pos(int) from file {}",
                  m_table_path + METADATA_FILE);
    return false;
  }

  m_metadata.first_deleted = first_deleted;
  return true;
}
