#include <cstring>
#include <ios>
#include <iostream>
#include <string>

// constexpr size_t ATTR_COUNT = 5;
//
// enum class TYPE { INT_T, VARCHAR_T };
//
// template <int T> struct t_getter {};
//
// auto get_attr(int pos) -> std::string {
//
//   static std::string attributes_name[ATTR_COUNT];
//   static TYPE attributes_type[ATTR_COUNT];
//
//   // int key_pos;
//   int attr_pos[ATTR_COUNT]; // pos of attribute in data
//   std::string buffer;       // data
//
//   int start = attr_pos[pos];
//   int end = attr_pos[pos + 1] - 1;
//
//   return buffer.substr(start, end - start);
// }

template <typename T> auto comp(std::string data, T key) -> bool {

  char *pr_data = data.data();
  T data_t;
  std::memcpy(&data_t, pr_data, sizeof(T));

  std::cout << "key " << data_t << std::endl;
  return data_t == key;
}

int main(int argc, char *argv[]) {

  int key = 4; // otro día

  int val_to_compare = 4;

  char saved_key[sizeof(int)];
  std::memcpy(saved_key, &val_to_compare, sizeof(int));

  int attr_pos = 0;
  std::string buffer; // data

  buffer = saved_key;

  std::cout << "are equal " << std::boolalpha << comp(buffer, key);
}
