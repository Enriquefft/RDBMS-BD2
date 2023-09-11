#include "HeapFile.hpp"
#include "Constants.hpp"
#include "Utils/File.hpp"

#include <spdlog/spdlog.h>
#include <utility>

constexpr int DEFAULT_DELETED = -1;

HeapFile::HeapFile(std::string table_name, std::vector<Type> types,
                   std::vector<std::string> attribute_names,
                   std::string primary_key)
    : m_table_name(std::move(table_name)),
      m_table_path(TABLES_PATH "/" + m_table_name + "/"),
      m_metadata(std::move(attribute_names), std::move(types),
                 std::move(primary_key)) {

  for (const auto &type : m_metadata.attribute_types) {
    C_RECORD_SIZE += type.size;
  }

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
