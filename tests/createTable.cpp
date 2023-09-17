#include <format>

#include <crow/app.h>
#include <crow/http_response.h>
#include <gtest/gtest.h>

#include "Api/Api.hpp"
#include "DBEngine.hpp"

class CreateTableTest : public ::testing::Test {
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
  void TearDown() override { DBEngine::drop_table(TEST_TABLE); }
};

TEST_F(CreateTableTest, BasicCreate) {

  const std::string QUERY = std::format(
      "CREATE TABLE {}(iden int primary key, col1 char(1), col2 char(4));",
      TEST_TABLE);

  auto response = test_request(QUERY);
  EXPECT_TRUE(engine.is_table(TEST_TABLE));
  EXPECT_EQ(response.code, 200);
}
