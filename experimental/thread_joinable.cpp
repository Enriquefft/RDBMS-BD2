#include <ios>
#include <iostream>
#include <thread>
int main(int argc, char *argv[]) {

  std::jthread t;

  {
    t = std::jthread([] { std::cout << "Hello from thread!\n"; });
  }

  std::cout << std::boolalpha << t.joinable() << '\n';

  return 0;
}
