#ifndef HEAP_FILE_HPP
#define HEAP_FILE_HPP

#include "Record/Record.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

class HeapFile {
public:
  using pos_type = std::streampos;

  explicit HeapFile(std::string table_name, std::vector<Type> types,
                    std::vector<std::string> attribute_names,
                    std::string primary_key);

  auto load() -> std::vector<Record>;
  auto add(const Record &record) -> pos_type;
  auto read(pos_type pos) -> Record;
  auto remove(pos_type pos) -> bool;

  auto get_type(const Attribute &attribute) -> Type;
  auto get_key(const Record &record) -> std::pair<Type, Attribute>;

  auto filter(const Record &record,
              const std::vector<std::string> &selected_attributes)
      -> std::string;
  auto filter(const std::vector<Record> &record,
              const std::vector<std::string> &selected_attributes)
      -> std::vector<std::string>;

  [[nodiscard]] auto get_attribute_names() const -> std::vector<std::string>;

private:
  using enum Record::Status;
  struct HeapRecord {
    Record record;
    Record::Status status = OK;
  };

  struct TableMetadata {
    std::vector<std::string> attribute_names;
    std::vector<Type> attribute_types;
    std::string primary_key;
    pos_type first_deleted{};
  };

  uint8_t C_RECORD_SIZE = 0;

  std::string m_table_name;
  std::string m_table_path;

  TableMetadata m_metadata;

  static std::fstream m_file_stream;

  void update_first_deleted(pos_type pos);
  auto read_metadata() -> bool;
  static void generate_deleted_record(char *deleted, int pos);
  [[nodiscard]] auto is_deleted(pos_type pos) const -> bool;
};

#endif // !HEAP_FILE_HPP
