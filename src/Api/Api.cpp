#include "Api.hpp"
#include <crow/common.h>
#include <cstdint>

constexpr std::uint16_t PORT = 8080;

Api::Api(DBEngine &db_engine, SqlParser &sql_parser)
    : m_db_engine(db_engine), m_sql_parser(sql_parser) {
  set_routes();
}

const auto PARSE_QUERY = [](const crow::request &req) {
  std::cout << "Received query request" << std::endl;

  crow::json::rvalue request_body = crow::json::load(req.body);

  // Extract the query string from the request body
  if (!request_body.has("query")) {
    return crow::response(crow::status::BAD_REQUEST, "Missing query");
  }
  std::string query = request_body["query"].s();

  // Dummy response object
  crow::json::wvalue response_body;
  response_body["existingTables"] = {"table1", "table2", "table3"};
  response_body["queryData"] = crow::json::wvalue::list();
  std::vector<std::string> columns = {"x column 1", "c column 2", "a column 3"};
  response_body["queryData"] = columns;

  crow::response response;
  // Set the response status code and content type
  response.code = crow::status::OK;
  response.set_header("Content-Type", "application/json");

  // Serialize the response object to JSON and send it as the response
  // body
  response.body = response_body.dump();
  return response;
};

void Api::set_routes() {

  CROW_ROUTE(m_app, "/query").methods(crow::HTTPMethod::POST)(PARSE_QUERY);

  m_app.validate(); // Used to make sure all the route handlers are in order.
}

void Api::run() {

  m_app.loglevel(crow::LogLevel::Info);
  m_app.port(PORT).multithreaded().run();
}

void Api::handle(crow::request &req, crow::response &res) {
  m_app.handle_full(req, res);
}
