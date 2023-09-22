#ifndef INDEX_HPP
#define INDEX_HPP

#include "data.hpp"
#include "response.hpp"
#include "utils.hpp"

template <typename KEY_TYPE> class Index {
protected:
  std::string attribute_name;
  std::string table_name;
  std::string index_name;
  std::string duplicatesFilename;
  bool PK;

  template <typename RecordType>
  void getAllRawCurrentRecords(RecordType sir,
                               std::vector<physical_pos> &records);

  template <typename FileType = std::fstream, typename RecordType>
  void insertDuplicate(FileType &file, RecordType &sir, RecordType &sir_dup);

  template <typename RecordType>
  void insertDuplicateFile(RecordType &sir, physical_pos &duplicate_position);

  template <typename HeaderType, typename RecordType>
  size_t numberRecordsWithHeader(std::string file_name);

  template <typename HeaderType, typename RecordType>
  void printFileWithHeader(std::string file_name);

  template <typename RecordType> size_t numberRecords(std::string file_name);

  template <typename RecordType> void printFile(const std::string &file_name);

  template <typename FileType = std::fstream, typename HeaderType>
  void writeHeader(FileType &file, HeaderType &header);

  template <typename FileType = std::fstream, typename HeaderType>
  void readHeader(FileType &file, HeaderType &header);

  template <typename FileType = std::fstream, typename RecordType>
  void readRecord(FileType &file, RecordType &record);

  template <typename FileType = std::fstream, typename RecordType>
  void writeRecord(FileType &file, RecordType &record);

  template <typename FileType = std::fstream, typename RecordType>
  void moveReadRecord(FileType &file, physical_pos &pos, RecordType &record);

  template <typename FileType = std::fstream, typename RecordType>
  void moveWriteRecord(FileType &file, physical_pos &pos, RecordType &record);

public:
  std::string get_attribute_name() { return this->attribute_name; }
  std::string get_table_name() { return this->table_name; }

  virtual std::string get_index_name() = 0;

  virtual Response add(Data<KEY_TYPE> data, physical_pos raw_pos) = 0;
  virtual Response search(Data<KEY_TYPE> data) = 0;
  virtual Response rangeSearch(Data<KEY_TYPE> begin, Data<KEY_TYPE> end) = 0;
  virtual Response erase(Data<KEY_TYPE> data) = 0;
};

#include "index.tpp"

#endif // INDEX_HPP
