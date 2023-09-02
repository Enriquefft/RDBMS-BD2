#include "Api/Api.hpp"
#include "DBEngine/DBEngine.hpp"
#include "SqlParser.hpp"

int main() {

  SqlParser parser;
  DBEngine engine;

  Api api(engine, parser);
  api.run();

  return 0;
}
