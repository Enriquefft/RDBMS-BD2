cmake_minimum_required(VERSION 3.26)

project(
  RDBMS
  VERSION 1.0
  DESCRIPTION "A relational database management system written in C++"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_executable(RDBMS src/main.cpp)

target_compile_features(RDBMS PRIVATE cxx_std_20)

target_compile_options(
  RDMS
  PRIVATE # Prefered warnings
          $<$<CXX_COMPILER_ID:MSVC>:/W4
          /WX>
          $<$<CXX_COMPILER_ID:Clang>:-Weverything>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
          -Wextra
          -Wpedantic
          -Werror>
          # Disable some warnings
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-c++98-compat>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-c++98-compat-pedantic>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-c++20-compat>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-zero-as-null-pointer-constant>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=padded>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
          $<$<CONFIG:RELEASE>:-Ofast>
          $<$<CONFIG:DEBUG>:-O0>
          $<$<CONFIG:DEBUG>:-ggdb3>
          "-fsanitize=address,undefined")

target_link_options(
  RDBMS PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

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
