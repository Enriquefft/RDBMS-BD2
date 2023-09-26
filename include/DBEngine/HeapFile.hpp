#ifndef HEAP_FILE_HPP
#define HEAP_FILE_HPP

#include "Record/Record.hpp"
#include "response.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <vector>

namespace DB_ENGINE {

class HeapFile {
public:
  using time_t = std::chrono::milliseconds;

  using pos_type = std::streampos;

  HeapFile() = delete;
  explicit HeapFile(std::string table_name, std::vector<Type> types,
                    std::vector<std::string> attribute_names,
                    std::string primary_key);
  explicit HeapFile(const std::filesystem::path &table_path);

  auto load() -> std::pair<std::vector<Record>, time_t>;
  auto add(const Record &record) -> pos_type;
  auto bulk_insert(const std::vector<Record> &records,
                   const std::vector<bool> &rec_to_insert) -> ::Index::Response;
  auto next_pos() const -> pos_type;

  auto read(const pos_type &pos) -> std::pair<Record, time_t>;
  auto read(const std::vector<pos_type> &pos)
      -> std::pair<std::vector<Record>, time_t>;
  auto remove(const pos_type &pos) -> std::pair<bool, time_t>;

  auto get_type(const std::string &attribute_name) const -> Type;
  auto get_type(const Attribute &attribute) const -> Type;
  auto get_types() const -> std::vector<Type>;

  auto get_key(const Record &record) const -> std::pair<Type, Attribute>;
  auto get_key_name() const -> std::pair<Type, std::string>;
  auto get_key_idx() const -> uint8_t;

  auto filter(Record &record,
              const std::vector<std::string> &selected_attributes,
              const query_time_t &times) const -> QueryResponse;
  auto filter(std::vector<Record> &record,
              const std::vector<std::string> &selected_attributes,
              const query_time_t &times) const -> QueryResponse;

  [[nodiscard]] auto get_attribute_names() const -> std::vector<std::string>;
  [[nodiscard]] auto get_index_names() const -> std::vector<std::string>;
  [[nodiscard]] auto get_attribute_idx(const std::string &attribute_name) const
      -> uint8_t {
    return m_metadata.get_attribute_idx(attribute_name);
  }
  [[nodiscard]] auto get_record_size() const -> uint8_t;

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
    pos_type first_deleted = -1;
    uint64_t record_count = 0;

    ~TableMetadata() {
      spdlog::info("TableMetadata destructor");
      spdlog::info("attribute_names size: {}", attribute_names.size());
    }

    TableMetadata(std::vector<std::string> _attribute_names,
                  std::vector<Type> _attribute_types, std::string _primary_key);
    TableMetadata() = default;

    [[nodiscard]] auto
    get_attribute_idx(const std::string &attribute_name) const -> uint8_t;
    auto record_size() -> uint64_t {
      return std::accumulate(
          attribute_types.begin(), attribute_types.end(),
          static_cast<uint8_t>(0),
          [](uint8_t sum, const Type &type) { return sum + type.size; });
    }
  };

  uint8_t C_RECORD_SIZE = 0;

  std::string m_table_name;
  std::string m_table_path;

  TableMetadata m_metadata;

  std::fstream m_file_stream;

  static auto string_cast(const Type &type, const char *data) -> std::string;

  auto rec_to_string(const Record &rec) -> std::string;

  void update_first_deleted(pos_type pos);
  auto read_metadata() -> bool;
  static void generate_deleted_record(char *deleted, int pos);
  [[nodiscard]] auto is_deleted(pos_type pos) const -> bool;
  void write_metadata();

  auto to_string() -> std::string {
    std::string str;
    str += "Table name: " + m_table_name + "\n";
    str += "Attribute names:\n";
    for (ulong i = 0; i < m_metadata.attribute_types.size(); i++) {
      str += m_metadata.attribute_names[i] + " ";
      str += m_metadata.attribute_types[i].to_string() + " ";
    }
    str += "Primary key: " + m_metadata.primary_key + "\n";
    return str;
  }
};

} // namespace DB_ENGINE

#endif // !HEAP_FILE_HPP
