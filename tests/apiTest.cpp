#include "Api/Api.hpp"
#include "DBEngine/DBEngine.hpp"
#include <crow/app.h>
#include <gtest/gtest.h>

class ApiTests : public ::testing::Test {

protected:
  SqlParser parser;
  DBEngine database;
  Api app;
};

TEST_F(ApiTests, Status200) {

  crow::request req;
  crow::response res;

  req.url = "/query";
  req.method = crow::HTTPMethod::POST;
  req.body = R"({"query": "SELECT * FROM users"})";

  app.handle(req, res);
  EXPECT_EQ(res.code, 200);
}

TEST_F(ApiTests, Status400) {
  crow::request req;
  crow::response res;

  req.url = "/query";
  req.method = crow::HTTPMethod::POST;
  req.body = R"({"not_query": "SELECT * FROM users"})";

  app.handle(req, res);
  EXPECT_EQ(res.code, 400);
}

TEST_F(ApiTests, Status404) {

  crow::request req;
  crow::response res;

  req.url = "/other_path";
  req.method = crow::HTTPMethod::POST;
  req.body = R"({"not_query": "SELECT * FROM users"})";

  app.handle(req, res);
  EXPECT_EQ(res.code, 404);
}
TEST_F(ApiTests, Status405) {

  crow::request req;
  crow::response res;

  req.url = "/query";
  req.method = crow::HTTPMethod::GET;
  req.body = R"({"not_query": "SELECT * FROM users"})";

  app.handle(req, res);
  EXPECT_EQ(res.code, 405);
}
