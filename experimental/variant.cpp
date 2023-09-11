#include <iostream>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

int main() {

  int x = 0;
  char p = '+';
  int y = 0;
  char m = '+';
  // More variables...

  static_assert(
      std::is_same_v<decltype(tuple), std::tuple<int, char, int, char, ...>>);

  return 0;
}
