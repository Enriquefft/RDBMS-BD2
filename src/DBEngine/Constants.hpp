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

constexpr std::array<std::string_view, 3> INDEX_TYPES = {"ISAM", "Sequential",
                                                         "OTROS xd"};
} // namespace Constants

#endif // CONSTANTS_HPP
