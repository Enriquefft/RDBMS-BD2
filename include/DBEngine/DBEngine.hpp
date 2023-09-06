#ifndef DB_ENGINE_HPP
#define DB_ENGINE_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "HeapFile.hpp"

template <uint8_t N> struct Varchar {
  char str[N];
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
  /// @brief The constructor of the class DBEngine.
  /// @details Constructs the class and all the necesary filepaths
  DBEngine();

  /// @brief Creates a new table.
  /// @tparam Atribute_types List of types corresponding to the attributes of
  /// the table
  /// @param table_name The name of the table to be created.
  /// @param attribute_names The names of the attributes of the table.
  /// @details Creates a new table with the name table_name. If the table
  /// already exists, it does nothing.
  template <typename... Atribute_types>
  auto create_table(
      const std::string_view &table_name,
      std::array<std::string, sizeof...(Atribute_types)> attribute_names)
      -> bool;

  /// @brief get a list of all the tables in the database.
  /// @return A vector of strings containing the names of all the tables.
  auto get_table_names() -> std::vector<std::string>;

  /// @brief Search for a key in a table.
  /// @param table_name The name of the table to search in.
  /// @param key The key to search for as a string.
  /// @return A vector of strings containing all the values associated with the
  /// key.
  auto search(std::string table_name, std::string key,
              std::function<bool(std::string)> expr)
      -> std::vector<std::string>;

  /// @brief Search for all the keys in a table that are in the range
  /// [begin_key, end_key].
  /// @param table_name The name of the table to search in.
  /// @param begin_key The begin key of the range.
  /// @param end_key The end key of the range.
  /// @return A vector of strings containing all the values associated with the
  /// key.
  auto range_search(std::string table_name,
                    std::pair<std::string, uint8_t> begin_key,
                    std::string end_key) -> std::vector<std::string>;

  /// @brief Add a new value to a table.
  /// @param table_name The name of the table to add the value to.
  /// @param value The value to add to the table.
  /// @return True if the value was added, false otherwise.
  auto add(std::string table_name, std::string value) -> bool;

  /// @brief Remove a value from a table.
  /// @param table_name The name of the table to remove the value from.
  /// @param key The key of the value to remove.
  /// @return True if the value was removed, false otherwise.
  auto remove(std::string table_name, std::string key) -> bool;

private:
  static void generate_directories();

  std::vector<HeapBase> m_tables_raw;

  auto get_indexes() -> std::vector<std::string>;
  auto get_table_attributes() -> std::vector<std::string>;
};

#endif // !DB_ENGINE_HPP
