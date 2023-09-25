#include <format>

#include <crow/app.h>
#include <crow/http_response.h>
#include <gtest/gtest.h>

#include "Api/Api.hpp"
#include "DBEngine.hpp"

constexpr bool REMOVE_AFTER_TEST = true;

class BulkInsertTest : public ::testing::Test {
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

TEST_F(BulkInsertTest, createTable) {

  const std::string TABLE_CREATE = std::format(
      "CREATE TABLE {}(id int primary key, nombre char(10), apellido "
      "char(20), aprobo_bd bool, score double);",
      TEST_TABLE);
  auto response = test_request(TABLE_CREATE);
  EXPECT_EQ(response.code, 200);
}

TEST_F(BulkInsertTest, insertSmalFile) {
  const std::string QUERY =
      std::format("insert into {} from 'students_insert';", TEST_TABLE);
  auto response = test_request(QUERY);
  EXPECT_EQ(response.code, 200);
}

TEST_F(BulkInsertTest, selectAll) {
  const std::string SELECT_QUERY = "SELECT * FROM test_table;";
  auto response = test_request(SELECT_QUERY);
  EXPECT_EQ(response.code, 200);
}

TEST_F(BulkInsertTest, dropTable) {
  if (!REMOVE_AFTER_TEST) {
    return;
  }
  auto query = "DROP TABLE " + TEST_TABLE + ";";
  auto response = test_request(query);
  EXPECT_EQ(response.code, 200);
}

TEST_F(BulkInsertTest, tableDropped) {
  if (!REMOVE_AFTER_TEST) {
    return;
  }
  EXPECT_FALSE(engine.is_table(TEST_TABLE));
}
