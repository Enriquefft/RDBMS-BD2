#include <fstream>

class container {
public:
  static std::fstream fs;

  void foo() const {
    container::fs.open("file.txt", std::ios::out);
    container::fs << "Hello, world!";
    container::fs.close();
  }
};

int main(int argc, char *argv[]) {
  container c;
  c.foo();
  return 0;
}
