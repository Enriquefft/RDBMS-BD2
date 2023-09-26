#include "Api.hpp"
#include "DBEngine.hpp"

#include <SqlParser.hpp>
#include <crow.h>
#include <crow/common.h>
#include <crow/json.h>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>

constexpr std::uint16_t PORT = 8080;

using crow::json::rvalue;
using crow::json::wvalue;

template <typename T>
  requires std::is_convertible_v<T, wvalue>
static auto vec_to_json(const std::vector<T> &vec) -> std::string {
  return static_cast<wvalue>(wvalue::list(vec.begin(), vec.end())).dump();
}

struct ApiResponse {

  wvalue::list tables;
  wvalue::list columns;
  wvalue::list rows;
  wvalue::object times;

  wvalue errror;
  wvalue code;

  ApiResponse(ParserResponse &&query_response) {

    if (query_response.failed()) {
      errror = query_response.error;
      code = query_response.code;
      return;
    }

    for (auto &table : query_response.table_names) {
      tables.emplace_back(std::move(table));
    }

    for (auto &column : query_response.column_names) {
      columns.emplace_back(std::move(column));
    }

    for (auto &[key, value] : query_response.query_times) {
      times[key] = value.count();
    }

    for (auto &record : query_response.records) {
      rows.emplace_back(wvalue::list(record.begin(), record.end()));
    }
  }

  [[nodiscard]] auto dump() -> std::string {

    wvalue response;
    response["tables"] = std::move(tables);
    response["columns"] = std::move(columns);
    response["rows"] = std::move(rows);
    response["times"] = std::move(times);
    return response.dump();
  }
};

Api::Api() { set_routes(); }

auto Api::parse_query(const crow::request &req) -> crow::response {

  spdlog::info("Recieved request");

  crow::json::rvalue request_body = crow::json::load(req.body);

  // Extract the query string from the request body
  if (!request_body.has("query")) {
    return {crow::status::BAD_REQUEST, "Missing query"};
  }

  std::string query = request_body["query"].s();

  spdlog::info("Request questy: {}", query);

  try {
    std::istringstream query_buffer(query);

    // response_body = m_sql_parser.parse(query_buffer);
    auto &engine = m_sql_parser.get_engine();
    ApiResponse api_response(std::move(m_sql_parser.parse(query_buffer)));

    m_sql_parser.clear();

    m_sql_parser.displayResponse();

    // Create the response
    crow::response response = api_response.dump();
    response.code = crow::status::OK;
    response.set_header("Content-Type", "application/json");

    return response;
  } catch (std::exception &e) {
    spdlog::error("Error parsing query: {}\n{}", query, e.what());
    return {crow::status::INTERNAL_SERVER_ERROR, {}};
  }
}

void Api::set_routes() {

  m_app.route<crow ::black_magic ::get_parameter_tag("/query")>("/query")
      .methods(crow::HTTPMethod::POST)(parse_query);
  m_app.validate(); // Used to make sure all the route handlers are in order.
}

void Api::run() {

  m_app.loglevel(crow::LogLevel::Info);
  m_app.port(PORT).multithreaded().run();
}

void Api::handle(crow::request &req, crow::response &res) {
  m_app.handle_full(req, res);
}
