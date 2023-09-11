#define t t
#define num 0

class Varchar {
  char *data;
  int size_of();
}

template <typename... T>
class record {
};

int main() { record<int, float> t; }
