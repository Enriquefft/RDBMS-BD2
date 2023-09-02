#ifndef API_HPP
#define API_HPP

#include "DBEngine/DBEngine.hpp"
#include "SqlParser.hpp"
#include "crow.h"

class Api {

public:
  Api(DBEngine &db_engine, SqlParser &sql_parser);

  Api() = delete;
  Api(const Api &) = delete;
  Api(Api &&) = delete;
  auto operator=(const Api &) -> Api & = delete;
  auto operator=(Api &&) -> Api & = delete;
  ~Api() = default;

  void run();

private:
  crow::SimpleApp m_app;

  // There is no issue with the members being references, as the Api class
  // doesn't allows copy, move or default construction.
  DBEngine &m_db_engine;
  SqlParser &m_sql_parser;
};

#endif // !API_HPP
