#include "Api.hpp"

#include <crow/common.h>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <stdexcept>

constexpr std::uint16_t PORT = 8080;

Api::Api(SqlParser &sql_parser) : m_sql_parser(sql_parser) { set_routes(); }

auto Api::parse_query(const crow::request &req) {
  std::cout << "Received query request" << std::endl;

  crow::json::rvalue request_body = crow::json::load(req.body);

  // Extract the query string from the request body
  if (!request_body.has("query")) {
    return crow::response(crow::status::BAD_REQUEST, "Missing query");
  }
  std::string query = request_body["query"].s();

  try {
    // Parse the query and populate the response body
    std::vector<std::string> response_body;
    m_sql_parser.parse(response_body, query);

    // Convert the response body to a JSON array
    crow::json::wvalue response_body_json;
    for (uint i = 0; i < response_body.size(); ++i) {
      // Populating the json like this enforces std::vector to be used in the
      // back
      response_body_json[i] = response_body[i];
    }

    // Create the response
    crow::response response;
    response.code = crow::status::OK;
    response.set_header("Content-Type", "application/json");
    response.body = response_body_json.dump();

    return response;
  } catch (std::exception &e) {
    spdlog::error("Error parsing query: {}", e.what());
    return crow::response(crow::status::BAD_REQUEST, e.what());
  }
}

void Api::set_routes() {

  CROW_ROUTE(m_app, "/query").methods(crow::HTTPMethod::POST)(parse_query);

  m_app.validate(); // Used to make sure all the route handlers are in order.
}

void Api::run() {

  m_app.loglevel(crow::LogLevel::Info);
  m_app.port(PORT).multithreaded().run();
}

void Api::handle(crow::request &req, crow::response &res) {
  m_app.handle_full(req, res);
}
