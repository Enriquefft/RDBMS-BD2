#include "avl_index.hpp"
#include "random"

void printResponse(Response &response) {
  std::cout << "-------" << std::endl;
  std::cout << "RESPONSE" << std::endl;
  for (auto &record : response.records) {
    std::cout << record << std::endl;
  }
  std::cout << "TIME: " << response.query_time.count() << " ms" << std::endl;
  std::cout << "-------" << std::endl;
}

template <typename KEY_TYPE>
void addNRandomRecord(AVLIndex<KEY_TYPE> &si, int N, KEY_TYPE min = 0,
                      KEY_TYPE max = 100) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(min, max);

  for (double i = 0; i < N; i++) {
    si.add(Data(i), 0);
  }
}

int main() {

  std::string table_name = "t1";
  std::string attribute_name = "attr1";
  bool PK = false;

  using attribute_type = double;

  // std::string indexFileName = "mi_indice.avl";
  // AVLIndex<attribute_type> index(indexFileName);

  Response res;

  AVLIndex<attribute_type> index(table_name, attribute_name, PK);
  // Data<attribute_type> item1, item2, item3, item4, item5, item6, item7,
  // item8, item9, item10;

  addNRandomRecord(index, 1000);

  Response response = index.rangeSearch(Data(100.0), Data(500.5));

  printResponse(response);

  /* item1.key = 1.1;
  item2.key = 2.2;
  item3.key = 3.3;
  item4.key = 4.4;
  item5.key = 5.5;
  item6.key = 6.6;
  item7.key = 7.7;
  item8.key = 8.8;
  item9.key = 9.9;
  item10.key = 4.4;


  res = index.add(item1, 1);
  printResponse(res);

  index.add(item2, 2);
  index.add(item3, 3);
  index.add(item4, 4);
  index.add(item5, 5);
  index.add(item6, 6);
  index.add(item7, 7);
  index.add(item8, 8);
  index.add(item9, 9);

  index.add(item10, 4);

  res = index.search(item4); */

  /*  index.printIndexFile();

   std::cout<<"*------------*"<<std::endl;
   index.printDuplicateFile(); */

  /* printResponse(res);

  index.displayPretty();

  std::vector<AVLIndexNode<attribute_type>> vector;

  res = index.rangeSearch(item3, item8);

  printResponse(res);

  res = index.search(item4);

  printResponse(res);

  index.erase(item1);
  index.erase(item3);
  index.erase(item6);
  res = index.erase(item4);

  printResponse(res);

  index.displayPretty(); */

  return 0;
}
