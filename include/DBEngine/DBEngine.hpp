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

#include "IndexManager/Indexes.hpp"

namespace DB_ENGINE {

/// @brief Enum representing the available Comparison operators
enum Comp : uint8_t { EQUAL, GE, LE, G, L };

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
  /// @brief Enum representing the available indexes
  enum class Index_t : uint8_t { AVL, HASH, ISAM };

  /// @brief The constructor of the class DBEngine.
  /// @details Constructs the class and all the necesary filepaths
  DBEngine();

  /// @brief Create a table with the given name, pk and attributes.
  /// @param table_name The name of the table to create.
  /// @param primary_key The name of the primary key.
  /// @param types The types of the attributes.
  /// @param attribute_names The names of the attributes.
  /// @return Boolean value indicating succesfull creation
  auto create_table(const std::string &table_name,
                    const std::string &primary_key,
                    const std::vector<Type> &types,
                    const std::vector<std::string> &attribute_names) -> bool;

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

  /// @brief Query if a table exists.
  /// @param table_name The name of the table to query.
  /// @return True if the table exists, false otherwise.
  auto is_table(const std::string &table_name) const -> bool;

  /// @brief Get the attributes of a table.
  /// @param table_name The name of the table to get the attributes from.
  /// @return A vector of strings containing the names of the attributes.
  auto get_table_attributes(const std::string &table_name) const
      -> std::vector<std::string>;

  /// @brief Get the Indexes asociated with a table.
  /// @param table_name The name of the table to get the indexes from.
  /// @return Vector of Index_t representing the indexes asociated with the
  /// table.
  auto get_indexes(const std::string &table_name) const -> std::vector<Index_t>;

  /// @brief Get the Indexes names asociated with a table.
  /// @param table_name The name of the table to get the indexes from.
  /// @return Vector of strings representing the attributes with indexes in the
  /// table.
  auto get_indexes_names(const std::string &table_name) const
      -> std::vector<std::string>;

  /// @brief Get a lambda function that compares a record's attribute with a
  /// value.
  /// @param table_name The name of the table to get the lambda from.
  /// @param cmp The comparison operator.
  /// @param column_name The name of the attribute to compare.
  /// @param string_to_compare The value to compare with.
  /// @return A predicate which operates on a record.
  auto get_comparator(const std::string &table_name, Comp cmp,
                      const std::string &column_name,
                      const std::string &string_to_compare) const
      -> std::function<bool(const Record &record)>;

  /// @brief Drop a table from the database.
  /// @param table_name The name of the table to drop.
  /// @details This function will delete the table, and all the indexes by
  /// removing the corresponding directory entries.
  static void drop_table(const std::string &table_name);

private:
  /// @brief Generate the directories necesary for the database.
  static void generate_directories();

  /// @brief Hashing struct which allows to have an unordered_map with std::pair
  /// as key
  /// @details Extracted from:
  /// https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/
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
  IndexMap<IsamIndexContainer> m_isam_indexes;
  IndexMap<SequentialIndexContainer> m_sequential_indexes;
  std::unordered_map<std::string, std::vector<Index_t>> m_index_map;

  static auto stob(std::string str) -> bool;
};

} // namespace DB_ENGINE

#endif // !DB_ENGINE_HPP
