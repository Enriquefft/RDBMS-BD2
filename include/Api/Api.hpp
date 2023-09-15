#ifndef API_HPP
#define API_HPP

#include "SqlParser.hpp"
#include "crow.h"

class Api {

public:
  Api();
  Api(const Api &) = delete;
  Api(Api &&) = delete;
  auto operator=(const Api &) -> Api & = delete;
  auto operator=(Api &&) -> Api & = delete;
  ~Api() = default;

  void run();
  void handle(crow::request &req, crow::response &res);

private:
  crow::SimpleApp m_app;

  // There is no issue with the members being references, as the Api class
  // doesn't allows copy, move or default construction.
  SqlParser m_sql_parser;

  void set_routes();
  static auto parse_query(const crow::request &req);
  std::function<crow::response(const crow::request &req)> m_parse_query;
};

#endif // !API_HPP
