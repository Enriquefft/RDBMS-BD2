#include <any>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>

template <uint32_t N> using Varchar = char[N];

template <typename Type> struct column_t {

  std::string name;
  bool is_pk;
  column_t(std::string _name, const bool &_is_pk)
      : name(std::move(_name)), is_pk(_is_pk) {}
};

template <typename... Attributes> class TableT {
public:
  std::tuple<column_t<Attributes>...> m_columns;
};

template <typename Type, typename... Attributes>
std::any add_column(column_t<Type> column, std::any table) {
  // Get the current table type
  using table_t =
      typename std::decay<decltype(std::any_cast<TableT<Attributes...>>(
          table))>::type;

  // Create a new table type with the added column
  using new_table_t = TableT<Attributes..., Type>;

  // Create a new table object with the added column
  new_table_t new_table;

  // Copy the existing columns from the old table to the new table
  std::apply(
      [&](auto &...columns) {
        std::tuple<column_t<Attributes>...> &existing_columns =
            new_table.m_columns;
        (existing_columns = std::make_tuple(columns...), ...);
      },
      std::any_cast<table_t>(table).m_columns);

  // Add the new column to the new table
  std::get<sizeof...(Attributes)>(new_table.m_columns) = column;

  // Return the new table as std::any
  return std::any(new_table);
}

int main() {

  std::any table = TableT<>();

  column_t<int> id("id", true);
  column_t<Varchar<5>> name("name", false);
  column_t<int> age("age", false);

  table = add_column(id, table);
  table = add_column(name, table);
  table = add_column(age, table);

  // table must store TableT<int, Varchar<5>, int>
  using table_t = table::type;

  return 0;
}
