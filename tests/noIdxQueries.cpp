#include <format>

#include <crow/app.h>
#include <crow/http_response.h>
#include <gtest/gtest.h>

#include "Api/Api.hpp"
#include "DBEngine.hpp"

constexpr bool REMOVE_AFTER_TEST = false;

class NoIdxQuery : public ::testing::Test {
protected:
  Api app;
  crow::request req;

  DBEngine &engine = Api::get_engine();

  std::string TEST_TABLE = "test_table";

  auto test_request(const std::string &query) -> crow::response {
    crow::response res;
    req.body = R"({"query": ")" + query + R"("})";
    app.handle(req, res);
    return res;
  }
  void SetUp() override {
    req.url = "/query";
    req.method = crow::HTTPMethod::POST;
  }
};

TEST_F(NoIdxQuery, createTable) {
  std::string query = "CREATE TABLE test(id int primary key, col1 char(50), "
                      "mode int, val double);";

  std::string query2 = "INSERT INTO test FROM 'data2 copy';";
  std::string query4 = "select * from test where mode < 3;";

  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(NoIdxQuery, insertData) {

  std::string query = "INSERT INTO test FROM 'data2 copy';";

  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(NoIdxQuery, selectModeL3) {
  std::string query = "select * from test where mode < 3;";

  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(NoIdxQuery, selectValG003) {
  std::string query = "select * from test where val > 0.03;";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(NoIdxQuery, dropTable) {
  if (!REMOVE_AFTER_TEST) {
    return;
  }
  auto query = "DROP TABLE " + TEST_TABLE + ";";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(NoIdxQuery, tableDropped) {
  if (!REMOVE_AFTER_TEST) {
    return;
  }
  EXPECT_FALSE(engine.is_table(TEST_TABLE));
}
