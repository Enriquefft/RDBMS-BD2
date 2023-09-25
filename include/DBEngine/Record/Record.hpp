#ifndef RECORD_HPP
#define RECORD_HPP

// This file contins compile time constants and type definitions
#include <limits>
#include <ranges>
#include <spdlog/spdlog.h>
#include <type_traits>
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

template <typename Func, typename... Args>
using func_return = std::invoke_result<
    Func,
    std::conditional_t<std::is_same_v<Args, std::string>, int, std::string>...>;

template <typename Func, typename... Args>
concept returns_void = std::is_same_v<func_return<Func, Args...>, void>;

namespace DB_ENGINE {

struct Attribute {
  std::string name;
  std::string value;
  friend auto operator<=>(const Attribute &, const Attribute &) = default;
};

struct Index {

  std::string table;
  std::string attribute_name;

  // spaceship operator
  friend auto operator<=>(const Index &, const Index &) = default;
};

struct Type {

  enum types : char { BOOL = 'b', INT = 'i', FLOAT = 'f', VARCHAR = 'c' };

  using size_type = uint16_t;

  static constexpr size_type MAX_VARCHAR_SIZE =
      std::numeric_limits<size_type>::max();

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
  [[nodiscard]] auto to_string() const -> std::string {
    std::string str;
    switch (type) {
    case BOOL:
      str += "BOOL";
      break;
    case INT:
      str += "INT";
      break;
    case FLOAT:
      str += "FLOAT";
      break;
    case VARCHAR:
      str += "VARCHAR";
      break;
    }
    str += "\tSize: " + std::to_string(size) + '\n';
    return str;
  }
};

namespace KEY_LIMITS {

// Ignore exit-time-destructors warning
const Attribute MIN = {"MIN", "MIN"};
const Attribute MAX = {"MAX", "MAX"};

} // namespace KEY_LIMITS

struct Record {
  using size_type = std::size_t;

  enum class Status : bool { DELETED = false, OK = true };

  Record() = default;
  Status status = Status::OK;

  explicit Record(auto split_view) {
    for (auto field : split_view) {
      m_fields.emplace_back(field.begin(), field.end());
    }
  }

  std::vector<std::string> m_fields;
  [[nodiscard]] auto begin() const { return m_fields.begin(); }
  [[nodiscard]] auto end() const { return m_fields.end(); }

  auto write(std::fstream &file, const std::vector<Type> &types) const
      -> std::ostream &;
  auto read(std::fstream &file, const std::vector<Type> &types)
      -> std::istream &;
  friend auto operator<=>(const Record &, const Record &) noexcept = default;
};

// https://en.cppreference.com/w/cpp/container/unordered_set
struct RecordHash {
  auto operator()(const Record &record) const -> size_t {
    size_t h = 0;
    for (const auto &field : record) {
      for (const auto &c : field) {
        h ^= std::hash<char>{}(c) + 0x9e3779b9 + (h << 6) + (h >> 2);
      }
    }
    return h;
  }
};

using query_time_t = std::unordered_map<std::string, std::chrono::milliseconds>;

struct QueryResponse {
  std::vector<Record> records;
  query_time_t query_times;
};

inline auto stob(std::string str) -> bool {

  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  static std::unordered_set<std::string> valid_strings = {
      "YES", "Y", "SI", "S", "V", "VERDADERO", "T", "TRUE", "1"};
  return valid_strings.contains(str);
}

template <typename Func, typename... Attributes>
  requires(std::convertible_to<Attributes, std::string> && ...)
inline void cast_and_execute(Type::types type, Func function,
                             const Attributes &...atts) {

  switch (type) {
  case Type::types::INT: {
    function(std::stoi(atts)...);
    break;
  }
  case Type::types::FLOAT: {
    function(std::stof(atts)...);
    break;
  }
  case Type::types::BOOL: {
    function(stob(atts)...);
    break;
  }
  case Type::types::VARCHAR: {
    function(atts...);
    break;
  }
  }
}
template <typename T = void, typename Func, typename... Attributes>
  requires(std::convertible_to<Attributes, std::string> && ...)
inline auto cast_and_execute(Type::types type, Func function,
                             const Attributes &...atts) -> T {

  switch (type) {
  case Type::types::INT: {
    return function(std::stoi(atts)...);
    break;
  }
  case Type::types::FLOAT: {
    return function(std::stof(atts)...);
    break;
  }
  case Type::types::BOOL: {
    return function(stob(atts)...);
    break;
  }
  case Type::types::VARCHAR: {
    return function(atts...);
    break;
  }
  }
}

// Keys can't be bool or varchar
template <typename Func, typename... Attributes>
  requires(std::convertible_to<Attributes, std::string> && ...)
inline void key_cast_and_execute(Type::types type, Func function,
                                 const Attributes &...atts) {

  switch (type) {
  case Type::types::BOOL: {
    spdlog::error("Attempting key conversion to bool");
    throw std::invalid_argument("Keys can't be bool");
  }
  case Type::types::VARCHAR: {
    spdlog::error("Attempting key conversion to string");
    throw std::invalid_argument("Keys can't be varchar");
  }
  case Type::types::INT: {
    function(std::stoi(atts)...);
    break;
  }
  case Type::types::FLOAT: {
    function(std::stof(atts)...);
    break;
  }
  }
}

template <typename Func, typename... Attributes>
inline auto key_cast_and_execute_ret(Type::types type, Func function,
                                     const Attributes &...atts)
    -> func_return<Func, Attributes...> {

  switch (type) {
  case Type::types::BOOL: {
    spdlog::error("Attempting key conversion to bool");
    throw std::invalid_argument("Keys can't be bool");
  }
  case Type::types::VARCHAR: {
    spdlog::error("Attempting key conversion to string");
    throw std::invalid_argument("Keys can't be varchar");
  }
  case Type::types::INT: {
    return function(std::stoi(atts)...);
    break;
  }
  case Type::types::FLOAT: {
    return function(std::stof(atts)...);
    break;
  }
  }
}

} // namespace DB_ENGINE

#endif // !RECORD_HPP
