cmake_minimum_required(VERSION 3.26)

project(
  RDBMS
  VERSION 1.0
  DESCRIPTION "A relational database management system written in C++"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# ##############################################################################
# Targets
add_executable(RDBMS src/main.cpp)

# add_subdirectory(src/Api) # Api
add_subdirectory(src/DBEngine) # DBEngine

# Needs to be set early to generate header files
add_subdirectory(libs/SqlParser) # SqlParser

# ##############################################################################
# library dependance
target_link_libraries(SqlParser PRIVATE DBEngine)
# target_link_libraries(Api PUBLIC SqlParser)
target_link_libraries(RDBMS PRIVATE SqlParser)

# ##############################################################################

target_include_directories(RDBMS PRIVATE include/DBEngine)

target_compile_features(RDBMS PRIVATE cxx_std_20)
target_compile_options(
  RDBMS
  PRIVATE # Prefered warnings
          $<$<CXX_COMPILER_ID:MSVC>:/W4
          /WX>
          $<$<CXX_COMPILER_ID:Clang>:-Weverything>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
          -Werror
          -Wall
          -Wextra
          -Wpedantic
          -Wformat=2
          -Wno-unused-parameter
          -Wshadow
          -Wwrite-strings
          -Wcast-qual
          -Wcast-align
          -Wconversion>
          # Disable some warnings when using clang's Weverything
          $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat-pedantic>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-compat>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-extensions>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-zero-as-null-pointer-constant>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-error=padded>
          $<$<CXX_COMPILER_ID:Clang>:-Wno-switch-enum>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
          $<$<CONFIG:RELEASE>:-Ofast>
          $<$<CONFIG:DEBUG>:-O0>
          $<$<CONFIG:DEBUG>:-ggdb3>)

target_link_options(
  RDBMS PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

target_link_libraries(DBEngine PRIVATE spdlog::spdlog)
target_link_libraries(RDBMS PRIVATE spdlog::spdlog)
target_link_libraries(SqlParser PRIVATE spdlog::spdlog)

get_target_property(SQLPARSER_LIBS SqlParser LINK_LIBRARIES)
get_target_property(SQLPARSER_INCLUDES SqlParser INCLUDE_DIRECTORIES)

get_target_property(DBENGINE_LIBS DBEngine LINK_LIBRARIES)
get_target_property(DBENGINE_INCLUDES DBEngine INCLUDE_DIRECTORIES)

# get_target_property(API_LIBS Api LINK_LIBRARIES)
# get_target_property(API_INCLUDES Api INCLUDE_DIRECTORIES)

message("SqlParser is linked with: ${SQLPARSER_LIBS}")
message("SqlParser include dirs: ${SQLPARSER_INCLUDES}")

message("Api is linked with: ${API_LIBS}")
message("Api include: ${API_INCLUDES}")

message("Engine is linked with: ${DBENGINE_LIBS}")
message("engine include: ${API_INCLUDES}")
# SpdLog
if(NOT TARGET spdlog) # Stand-alone build
  find_package(spdlog REQUIRED)
endif()

# Ctest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git # Always fetch the
                                                          # latest version
  GIT_TAG main)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
  option(PACKAGE_TESTS "Build the tests" ON)
endif()

if(PACKAGE_TESTS)
  enable_testing()
  include(GoogleTest)
  # add_subdirectory(tests)
endif()
