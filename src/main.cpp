#include "Api/Api.hpp"
#include "DBEngine/DBEngine.hpp"
#include "SqlParser.hpp"
#include "spdlog/spdlog.h"

int main() {

  SqlParser parser;
  DBEngine engine;

  Api api(engine, parser);
  spdlog::info("Welcome to spdlog!");
  api.run();

  return 0;
}
