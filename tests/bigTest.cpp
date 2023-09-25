

#include <crow/app.h>
#include <crow/http_response.h>
#include <gtest/gtest.h>

#include "Api/Api.hpp"
#include "DBEngine.hpp"

class ElRealTest : public ::testing::Test {
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

TEST_F(ElRealTest, create) {
  auto query =
      "CREATE TABLE " + TEST_TABLE +
      " (count double, id int primary key, name char(12), paso_bd bool);";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(ElRealTest, insertSingle) {
  auto query =
      "INSERT INTO " + TEST_TABLE + " Values (3.5, 8, 'qwerty', 'true');";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}
TEST_F(ElRealTest, insertMore) {
  auto query1 =
      "INSERT INTO " + TEST_TABLE + " Values (3.5, 10, 'qwerty', 'true');";
  auto query2 =
      "INSERT INTO " + TEST_TABLE + " Values (3.5, 9, 'qwerty', 'true');";
  auto response1 = test_request(query1);
  auto response2 = test_request(query2);
  EXPECT_EQ(response1.code, 200);
  EXPECT_EQ(response2.code, 200);
}

TEST_F(ElRealTest, selectAll) {
  auto query = "SELECT * FROM " + TEST_TABLE + ";";
  auto response = test_request(query);
  spdlog::info("{}", response.body);
  EXPECT_EQ(response.code, 200);
}

TEST_F(ElRealTest, selectSome) {
  auto query = "SELECT name, id FROM " + TEST_TABLE + ";";
  auto response = test_request(query);
  spdlog::info("{}", response.body);
  EXPECT_EQ(response.code, 200);
}

TEST_F(ElRealTest, dropTable) {
  auto query = "DROP TABLE " + TEST_TABLE + ";";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(ElRealTest, failedInsert) {
  auto query =
      "INSERT INTO " + TEST_TABLE + " Values (3.5, 8, 'qwerty', 'true');";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 400);
}
