#ifndef HEAP_FILE_HPP
#define HEAP_FILE_HPP

#include "Record/Record.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <vector>

class HeapFile {
public:
  using pos_type = std::streampos;

  HeapFile() = default;
  explicit HeapFile(std::string table_name, std::vector<Type> types,
                    std::vector<std::string> attribute_names,
                    std::string primary_key);

  auto load() -> std::vector<Record>;
  auto add(const Record &record) -> pos_type;
  auto read(const pos_type &pos) -> Record;
  auto remove(const pos_type &pos) -> bool;

  auto get_type(const std::string &attribute_name) const -> Type;
  auto get_type(const Attribute &attribute) const -> Type;

  auto get_key(const Record &record) const -> std::pair<Type, Attribute>;

  auto filter(const Record & /*record*/,
              const std::vector<std::string> & /*selected_attributes*/) const
      -> std::string {
    return {};
  }
  auto filter(const std::vector<Record> & /*record*/,
              const std::vector<std::string> & /*selected_attributes*/) const
      -> std::vector<std::string> {
    return {};
  }

  [[nodiscard]] auto get_attribute_names() const -> std::vector<std::string>;
  [[nodiscard]] auto get_index_names() const -> std::vector<std::string>;
  [[nodiscard]] auto get_attribute_idx(const std::string &attribute_name) const
      -> uint8_t {
    return m_metadata.get_attribute_idx(attribute_name);
  }

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

    [[nodiscard]] auto
    get_attribute_idx(const std::string &attribute_name) const -> uint8_t;
    [[nodiscard]] auto record_count() const -> std::size_t {
      return attribute_types.size();
    }
    auto size() -> uint64_t {
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

  void update_first_deleted(pos_type pos);
  auto read_metadata() -> bool;
  static void generate_deleted_record(char *deleted, int pos);
  [[nodiscard]] auto is_deleted(pos_type pos) const -> bool;
};

#endif // !HEAP_FILE_HPP
