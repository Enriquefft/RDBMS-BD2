#include "Api/Api.hpp"
#include "DBEngine/DBEngine.hpp"
#include "SqlParser.hpp"

int main(int argc, char *argv[]) {

  SqlParser parser;
  DBEngine engine;

  Api api(parser, engine);
  api.run();

  return 0;
}
