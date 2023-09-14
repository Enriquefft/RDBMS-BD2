#ifndef RECORD_HPP
#define RECORD_HPP

// This file contins compile time constants and type definitions
#ifdef __clang__
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <string>
#include <unordered_set>
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
  Type(const types &_type, const int &_size)
      : Type(_type, static_cast<size_type>(_size)) {}
};

namespace KEY_LIMITS {

// Ignore exit-time-destructors warning
const Attribute MIN = {"MIN", "MIN"};
const Attribute MAX = {"MAX", "MAX"};

} // namespace KEY_LIMITS

struct Record {

  enum class Status : bool { DELETED = false, OK = true };

  Record() = default;

  std::vector<char *> fields;

  void write(std::fstream &file, const std::vector<Type> &types) {}
  auto read(std::fstream &file, const std::vector<Type> &types) -> Record {
    return {};
  }
};

inline auto stob(std::string str) -> bool {

  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  static std::unordered_set<std::string> valid_strings = {
      "YES", "Y", "SI", "S", "V", "VERDADERO", "T", "TRUE"};
  return valid_strings.contains(str);
}

template <typename Func>
inline void cast_and_execute(Type::types type,
                             const std::string &attribute_value, Func func) {
  switch (type) {
  case Type::types::INT: {
    break;
  }
  case Type::types::FLOAT: {
    float key_value = std::stof(attribute_value);
    func(key_value);
    break;
  }
  case Type::types::BOOL: {
    bool key_value = stob(attribute_value);
    func(key_value);
    break;
  }
  case Type::types::VARCHAR: {
    func(attribute_value);
    break;
  }
  }
}

template <typename Func>
inline void cast_and_execute(Type::types type, const std::string &att1,
                             const std::string &att2, Func func) {
  switch (type) {
  case Type::types::INT: {
    int value_1 = std::stoi(att1);
    int value_2 = std::stoi(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::FLOAT: {
    float value_1 = std::stof(att1);
    float value_2 = std::stof(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::BOOL: {
    bool value_1 = stob(att1);
    bool value_2 = stob(att2);
    func(value_1, value_2);
    break;
  }
  case Type::types::VARCHAR: {
    func(att1, att2);
    break;
  }
  }
}

#endif // !RECORD_HPP
