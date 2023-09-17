#include "Api/Api.hpp"
#include <crow/app.h>
#include <crow/http_response.h>
#include <gtest/gtest.h>

class CreateTableTest : public ::testing::Test {
protected:
  Api app;
  crow::request req;
  void SetUp() override {
    req.url = "/query";
    req.method = crow::HTTPMethod::POST;
  }
  auto test_request(const std::string &query) -> crow::response {
    crow::response res;
    req.body = R"({"query": ")" + query + R"("})";
    app.handle(req, res);
    return res;
  }
};

TEST_F(CreateTableTest, BasicCreate) {

  const std::string QUERY = "CREATE TABLE basic_create(iden int primary key, "
                            "col1 char(1), col2 char(4));";
  auto response = test_request(QUERY);
  EXPECT_EQ(response.code, 200);
}
