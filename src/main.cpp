#include "Api/Api.hpp"
#include "DBEngine/DBEngine.hpp"
#include "SqlParser.hpp"
#include "spdlog/spdlog.h"

int main() {

  DBEngine engine;
  // SqlParser parser(engine);
  SqlParser parser;

  Api api(parser);

  api.run();

  return 0;
}
