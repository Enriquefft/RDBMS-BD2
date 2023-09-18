#ifndef DB_ENGINE_HPP
#define DB_ENGINE_HPP

#include <cstdint>
#include <functional>
#include <ios>
#include <spdlog/spdlog.h>
#include <string>
#include <variant>
#include <vector>

#include "HeapFile.hpp"

namespace DB_ENGINE {

enum Comp : uint8_t { EQUAL, GE, LE, G, L };

template <typename T> class AVLIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }

  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in AVL index", begin, end);
    return {};
  }

  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from AVL index", key);
    return {};
  }

  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in AVL index", key);

    return {};
  }

  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to AVL index with pos {}", key, pos);
    return {};
  }
};

class ISAMIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }
  template <typename T>
  auto range_search(T /*begin*/, T /*end*/) const
      -> std::vector<std::streampos> {
    return {};
  }
  template <typename T> auto remove(T /*key*/) const -> std::streampos {
    return {};
  }
  template <typename T> auto search(T /*key*/) const -> std::streampos {
    return {};
  }
  template <typename T>
  auto add(T /*key*/, std::streampos /*pos*/) const -> bool {
    return {};
  }
};

class SequentialIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }
  template <typename T>
  auto range_search(T /*begin*/, T /*end*/) const
      -> std::vector<std::streampos> {
    return {};
  }
  template <typename T> auto remove(T /*key*/) const -> std::streampos {
    return {};
  }
  template <typename T> auto search(T /*key*/) const -> std::streampos {
    return {};
  }
  template <typename T>
  auto add(T /*key*/, std::streampos /*pos*/) const -> bool {
    return {};
  }
};

class AvlIndexContainer {
  std::variant<AVLIndex<int>, AVLIndex<float>, AVLIndex<std::string>,
               AVLIndex<bool>>
      m_idx;
};

/**
 * class DBEngine
 * @brief The class DBEngine is the public interface for the database engine.
 *
 * @details The class DBEngine contains the public interface for the database
 * operations, which then are called on the per-file file managers (raw, isam,
 * etc).
 */
class DBEngine {
public:
  enum class Index_t { AVL, HASH, ISAM };

  /// @brief The constructor of the class DBEngine.
  /// @details Constructs the class and all the necesary filepaths
  DBEngine();

  auto create_table(std::string table_name, const std::string &primary_key,
                    std::vector<Type> types,
                    std::vector<std::string> attribute_names) -> bool;

  /// @brief get a list of all the tables in the database.
  /// @return A vector of strings containing the names of all the tables.
  auto get_table_names() -> std::vector<std::string>;

  /// @brief Search for a key in a table.
  /// @param table_name The name of the table to search in.
  /// @param key The key to search for as a string.
  /// @return A vector of strings containing all the values associated with the
  /// key.
  auto search(const std::string &table_name, const Attribute &key,
              const std::function<bool(Record)> &expr,
              const std::vector<std::string> &selected_attributes)
      -> std::string;

  /// @brief Search for all the keys in a table that are in the range
  /// [begin_key, end_key].
  /// @param table_name The name of the table to search in.
  /// @param begin_key The begin key of the range.
  /// @param end_key The end key of the range.
  /// @return A vector of strings containing all the values associated with the
  /// key.
  auto range_search(const std::string &table_name, const Attribute &begin_key,
                    const Attribute &end_key,
                    const std::function<bool(Record)> &expr,
                    const std::vector<std::string> &selected_attributes)
      -> std::vector<std::string>;

  /// @brief Add a new value to a table.
  /// @param table_name The name of the table to add the value to.
  /// @param value The value to add to the table.
  /// @return True if the value was added, false otherwise.
  auto add(const std::string &table_name, const Record &value) -> bool;

  /// @brief Remove a value from a table.
  /// @param table_name The name of the table to remove the value from.
  /// @param key The key of the value to remove.
  /// @return True if the value was removed, false otherwise.
  auto remove(const std::string &table_name, const Attribute &key) -> bool;

  auto is_table(const std::string &table_name) const -> bool;
  auto get_table_attributes(const std::string &table_name) const
      -> std::vector<std::string>;

  auto get_indexes(const std::string &table_name) const -> std::vector<Index_t>;
  auto get_indexes_names(const std::string &table_name) const
      -> std::vector<std::string>;

  auto get_comparator(const std::string &table_name, Comp cmp,
                      const std::string &column_name,
                      const std::string &string_to_compare) const
      -> std::function<bool(const Record &record)>;
  static void drop_table(const std::string &table_name);

private:
  static void generate_directories();

  // Extracted from:
  // https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/
  struct HashPair {
    auto operator()(const Index &index) const -> size_t {
      auto hash1 = std::hash<std::string>{}(index.table);
      auto hash2 = std::hash<std::string>{}(index.attribute_name);
      if (hash1 != hash2) {
        return hash1 ^ hash2;
      }
      // If hash1 == hash2, their XOR is zero.
      return hash1;
    }
  };

  template <typename T> using IndexMap = std::unordered_map<Index, T, HashPair>;

  template <typename T> using TableMap = std::unordered_map<std::string, T>;

  TableMap<HeapFile> m_tables_raw;
  IndexMap<AvlIndexContainer> m_avl_indexes;
  IndexMap<ISAMIndex> m_isam_indexes;
  IndexMap<SequentialIndex> m_sequential_indexes;
  std::unordered_map<std::string, std::vector<Index_t>> m_index_map;

  static auto stob(std::string str) -> bool;

  // template <typename Func>
  // void cast_and_execute(Type::types type, const std::string &attribute_value,
  //                       Func func);
  // template <typename Func>
  // void cast_and_execute(Type::types type, const std::string &att1,
  //                       const std::string &att2, Func func);
};

} // namespace DB_ENGINE

#endif // !DB_ENGINE_HPP
