cmake_minimum_required(VERSION 3.26)

project(
  RDBMS
  VERSION 1.0
  DESCRIPTION "A relational database management system written in C++"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Needs to be set before add_subdirectory to generate header files
add_subdirectory(libs/SqlParser)

add_executable(RDBMS src/main.cpp)

target_link_libraries(RDBMS PUBLIC Api DBEngine sql_parser)
target_include_directories(RDBMS PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src/Api)
add_subdirectory(src/DBEngine)

target_compile_features(RDBMS PRIVATE cxx_std_20)

target_compile_options(
  RDBMS
  INTERFACE # Prefered warnings
            $<$<CXX_COMPILER_ID:MSVC>:/W4
            /WX>
            $<$<CXX_COMPILER_ID:Clang>:-Weverything>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror
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
            $<$<CXX_COMPILER_ID:Clang>:-Wno-zero-as-null-pointer-constant>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-error=padded>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
            $<$<CONFIG:RELEASE>:-Ofast>
            $<$<CONFIG:DEBUG>:-O0>
            $<$<CONFIG:DEBUG>:-ggdb3>
            "-fsanitize=address,undefined")

target_link_options(
  RDBMS PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

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
