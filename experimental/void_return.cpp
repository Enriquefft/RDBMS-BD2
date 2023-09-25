#include <concepts>
#include <iostream>
#include <ostream>
#include <string>

template <typename... Args> void foo(Args... /*args*/) {
  std::tuple<std::conditional_t<std::is_same_v<Args, std::string>, int,
                                std::string>...>
      tup;

  if constexpr (std::is_same_v<decltype(tup), std::tuple<int, int>>) {
    std::cout << "int int" << std::endl;
  }
  if constexpr (std::is_same_v<decltype(tup), std::tuple<int, int, int>>) {
    std::cout << "int int int" << std::endl;
  }
}

int main() {

  foo<std::string, std::string, std::string>("a", "b", "v");
  foo<std::string, std::string>("b", "v");

  return 0;
}
