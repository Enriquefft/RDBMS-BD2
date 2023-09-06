#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>
#include <string>

namespace Constants {

// Must be owning std::strings to allow for concatenation
#define BASE_PATH "DB_FILES"
#define TABLES_PATH BASE_PATH "/Tables/"
#define METADATA_PATH BASE_PATH "/Metadata"
#define INDEX_PATH BASE_PATH "/Indexes"

#define DATA_FILE "data.bin"
#define METADATA_FILE "metadata.bin"

constexpr std::array INDEX_TYPES = {"ISAM", "Sequential", "AVL"};

} // namespace Constants

#endif // CONSTANTS_HPP
