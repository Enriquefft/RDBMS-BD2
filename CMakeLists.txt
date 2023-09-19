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

add_subdirectory(src/Api) # Api
add_subdirectory(src/DBEngine) # DBEngine
add_subdirectory(libs/SqlParser) # SqlParser
# add_subdirectory(libs/SequentialIndex)

# ##############################################################################
# library dependance
target_link_libraries(SqlParser PUBLIC DBEngine)
target_link_libraries(Api PUBLIC SqlParser)
target_link_libraries(RDBMS PRIVATE Api)

# Indexes target_link_libraries(DBEngine PUBLIC SequentialIndex)

# ##############################################################################

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)
target_compile_options(
  common
  INTERFACE # Prefered warnings
            $<$<CXX_COMPILER_ID:MSVC>:/W4
            /WX>
            $<$<CXX_COMPILER_ID:Clang>:-Weverything>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            -Werror
            -Wall
            -Wextra
            -Wpedantic
            -Wformat=2
            -Wshadow
            -Wwrite-strings
            -Wcast-qual
            -Wcast-align
            -Wconversion> #
            # Disable some warnings when using clang's Weverything
            $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat-pedantic>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-compat>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-extensions>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-c99-extensions>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-zero-as-null-pointer-constant>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-error=padded> #
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined> #
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
            $<$<CONFIG:RELEASE>:-Ofast>
            $<$<CONFIG:DEBUG>:-O0>
            $<$<CONFIG:DEBUG>:-ggdb3>)

target_link_options(
  common INTERFACE
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

# config
target_link_libraries(RDBMS PRIVATE common)
target_link_libraries(Api PRIVATE common)
target_link_libraries(DBEngine PRIVATE common)

# SpdLog

find_package(spdlog QUIET REQUIRED)

if(spdlog_FOUND)
  target_link_libraries(DBEngine PUBLIC spdlog::spdlog)
else()
  # Only use this if spdlog is not installed on your system
  target_include_directories(DBEngine SYSTEM
                             PUBLIC ${CMAKE_SOURCE_DIR}/libs/SpdLog/)
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
FetchContent_MakeAvailable(googletest)

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
  option(PACKAGE_TESTS "Build the tests" ON)
endif()

if(PACKAGE_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()
