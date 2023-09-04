#include <filesystem>
#include <ranges>

#include <spdlog/spdlog.h>

#include "Constants.hpp"
#include "DBEngine.hpp"

using std::filesystem::create_directory;
using std::filesystem::exists;

DBEngine::DBEngine() {
  generate_filepaths();

  //
}

auto DBEngine::create_table(const std::string &table_name) -> bool {
  // Check if table already exists
  if (exists("DB_FILES/Tables/" + table_name)) {
    return false;
  }

  //

  return true;
}

void DBEngine::generate_filepaths() {

  std::string base_path(BASE_PATH);

  bool created_base_dir = create_directory(base_path);
  spdlog::info(created_base_dir ? "Initialized filesystem"
                                : "Loaded filesystem");

  // Tables metadata
  if (create_directory(TABLES_PATH)) {
    spdlog::info("Created Tables subdir");
  }

  // Metadata
  if (create_directory(METADATA_PATH)) {
    spdlog::info("Created Metadata subdir");
  }

  // Indexes
  if (create_directory(INDEX_PATH)) {
    spdlog::info("Created Indexes subdir");
  }

  // Create subdirs for each index type
  for (const std::string_view &idx_name : Constants::INDEX_TYPES) {

    std::string name(idx_name);
    if (create_directory(INDEX_PATH "/" + name)) {
      spdlog::info("Created {} subdir", idx_name);
    }
  }

  spdlog ::info("DBEngine created");
}
