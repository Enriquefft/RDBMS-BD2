
#include <string>
enum class types { INT, FLOAT, DOUBLE, STRING };

auto main() -> int {

  std::string inknown_type = "4.5";
  types type = types::STRING;

  switch (type) {

  case types::INT:
  case types::FLOAT:
  case types::DOUBLE:
  case types::STRING:
    break;
  }

  return 0;
}
