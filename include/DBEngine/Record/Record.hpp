#ifndef RECORD_HPP
#define RECORD_HPP

#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <string>
#include <vector>

struct Attribute {
  std::string name;
  std::string value;
};

struct Index {

  std::string table;
  std::string attribute_name;

  // spaceship operator
  auto operator<=>(const Index &) const = default;
};

struct Type {

  enum types : char { BOOL = 'b', INT = 'i', FLOAT = 'f', VARCHAR = 'c' };

  using size_type = std::size_t;

  size_type size;
  types type;

  Type() = default;
  Type(const types &_type, const size_type &_size) : size(_size), type(_type) {}
  explicit Type(const types &_type) : type(_type) {

    switch (type) {
    case BOOL:
      this->size = sizeof(bool);
      break;
    case INT:
      this->size = sizeof(int);
      break;
    case FLOAT:
      this->size = sizeof(float);
      break;
    case VARCHAR:
      throw std::invalid_argument(
          "Type initialization from varchar must have a size");
    }
  }
};

struct Record {

  enum class Status : bool { DELETED = false, OK = true };
  Record(std::initializer_list<char *>);
  std::vector<char *> fields;

  void write(std::fstream &file, const std::vector<Type> &types);
  auto read(std::fstream &file, const std::vector<Type> &types) -> Record;
};

#endif // !RECORD_HPP
