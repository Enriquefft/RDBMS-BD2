#include "DBEngine.hpp"
#include <filesystem>

#include "spdlog/spdlog.h"

DBEngine::DBEngine() {

  bool exists = std::filesystem::create_directory("DB_FILES");

  spdlog::info("DBEngine created");
}

void DBEngine::create_table(const std::string &table_name) {

  // Check if table already exists
}
