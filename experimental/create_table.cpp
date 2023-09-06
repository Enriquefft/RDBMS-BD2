
#include <array>
#include <cstdint>
#include <iostream>
#include <string_view>
template <typename... Atribute_types>
auto create_table(
    const std::string_view &table_name,
    std::array<std::string, sizeof...(Atribute_types)> attribute_names)
    -> bool {

  std::cout << "TABLE NAME: " << table_name << "\n";
  for (ulong i = 0; i < sizeof...(Atribute_types); i++) {
    std::cout << attribute_names[i];
  }
  return true;
}

template <uint8_t N> struct Varchar {
  char str[N];
};

int main() {

  create_table<int, char, Varchar<5>>("table name", {"id", "gender", "name"});
  return 0;
}
