#include <fstream>
#include <iostream>

template <typename T = std::fstream> void foo() {
  std::cout << "foo" << std::endl;
}

int main(int argc, char *argv[]) {
  std::fstream file;
  foo<std::ifstream>();

  return 0;
}
