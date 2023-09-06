#include <filesystem>
#include <ranges>

#include <spdlog/spdlog.h>

#include "Constants.hpp"
#include "DBEngine.hpp"

using std::filesystem::create_directory;
using std::filesystem::exists;

DBEngine::DBEngine() {
  generate_directories();

  // Read tables raw data

  // Load Indexes
}

template <typename... Atribute_types>
auto DBEngine::create_table(const std::string_view &table_name,
                            std::vector<std::string> attribute_names) -> bool {
  // Check if table already exists

  std::string table_path = strcat(TABLES_PATH, table_name.data());

  if (exists(table_path)) {
    spdlog::warn("Table {} already exists.", table_name);
    return false;
  }
  create_directory(table_path);

  m_tables_raw.emplace(table_name);

  return true;
}

void DBEngine::generate_directories() {

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
  for (const auto &idx_name : Constants::INDEX_TYPES) {

    // std::string name(idx_name);
    if (create_directory(strcat(INDEX_PATH "/", idx_name))) {
      spdlog::info("Created {} subdir", idx_name);
    }
  }

  spdlog ::info("DBEngine created");
}
