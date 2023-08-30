#include <string>

constexpr size_t ATTR_COUNT = 5;

enum class TYPE { INT_T, VARCHAR_T };

class Buffer {

  static std::string attributes_name[ATTR_COUNT];
  static TYPE attributes_type[ATTR_COUNT];

  // int key_pos;
  int attr_pos[ATTR_COUNT]; // pos of attribute in data
  std::string buffer;       // data

  template <typename T> auto get_attr(int pos) -> T {

    int start = attr_pos[pos];
    int end = attr_pos[pos + 1] - 1;

    std::string attr = buffer.substr(start, end - start);

    switch (attributes_type[pos]) {
    case TYPE::VARCHAR_T:
      return attr;
    case TYPE::INT_T:
      return stoi(attr);
    }
  }

  std::string to_json() { return "{}"; }

  friend auto operator<<(std::ofstream &out, const Buffer &buf)
      -> std::ofstream &;
  friend auto operator<<(std::fstream &out, const Buffer &buf)
      -> std::fstream &;

  friend auto operator>>(std::fstream &out, const Buffer &buf)
      -> std::fstream &;
  friend auto operator>>(std::ifstream &out, const Buffer &buf)
      -> std::istream &;
};
