#include <any>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <variant>

class Varchar {
  char *m_data;
  uint8_t m_size;
};

template <typename Type> struct column_t {

  std::string name;
  bool is_pk;
  column_t(std::string _name, const bool &_is_pk)
      : name(std::move(_name)), is_pk(_is_pk) {}
};

template <typename Type> using col = column_t<Type>;
using col_types =
    std::variant<col<char>, col<Varchar>, col<int>, col<float>, col<bool>>;

template <typename... Attributes> class TableT {
public:
  std::tuple<column_t<Attributes>...> m_columns;

  using type = std::tuple<Attributes...>;

  explicit TableT(Attributes... _columns) : m_columns(std::move(_columns)...) {}
  explicit TableT(std::tuple<column_t<Attributes>...> _columns)
      : m_columns(std::move(_columns)) {}
};

template <typename newAtt, typename... Attributes>
auto add_column(column_t<newAtt> col, TableT<Attributes...> table)
    -> TableT<newAtt, Attributes...> {

  std::tuple<column_t<newAtt>> new_elem(col);
  // join col + table.m_columns
  auto new_tup = std::tuple_cat(new_elem, table.m_columns);

  return TableT<newAtt, Attributes...>(new_tup);
}

#define LAST_COL tlast

int main() {

  table;

  return 0;
}
