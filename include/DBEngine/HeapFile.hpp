#ifndef HEAP_FILE_HPP
#define HEAP_FILE_HPP

#include "Record/Record.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

enum class RecordStatus : bool { DELETED = false, OK = true };

class HeapBase {};

template <typename... Atribute_types> class HeapFile : HeapBase {

  using record_t = Record<Atribute_types...>;

  struct HeapRecord {
    record_t record;
    RecordStatus status = RecordStatus::OK;
  };

public:
  using pos_type = std::streampos;

  explicit HeapFile(
      std::string table_name,
      std::array<std::string, sizeof...(Atribute_types)> attribute_names);

  auto load() -> std::vector<record_t>;
  auto add(const Record<record_t> &record) -> pos_type;
  auto read(pos_type pos) -> record_t;
  auto remove(pos_type pos) -> bool;

private:
  const uint8_t C_ATRIBUTE_COUNT = sizeof...(Atribute_types);
  const uint8_t C_RECORD_SIZE = sizeof(HeapRecord);

  std::string m_table_name;
  std::string m_table_path;
  std::array<std::string, sizeof...(Atribute_types)> m_attribute_names;

  static std::fstream m_file_stream;
  pos_type m_first_deleted{};

  void update_first_deleted(pos_type pos);
  auto read_metadata() -> bool;
  static void generate_deleted_record(char *deleted, int pos);
  [[nodiscard]] auto is_deleted(pos_type pos) const -> bool;
};

#endif // !HEAP_FILE_HPP
