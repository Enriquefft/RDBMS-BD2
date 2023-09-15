cmake_minimum_required(VERSION 3.26)

add_library(DBEngine DBEngine.cpp)

target_sources(DBEngine PRIVATE HeapFile.cpp Record/Record.cpp)

target_include_directories(DBEngine PUBLIC ${CMAKE_SOURCE_DIR}/include/DBEngine)

target_compile_features(DBEngine PRIVATE cxx_std_20)
target_compile_options(
  DBEngine
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
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
          $<$<CONFIG:RELEASE>:-Ofast>
          $<$<CONFIG:DEBUG>:-O0>
          $<$<CONFIG:DEBUG>:-ggdb3>)

target_link_options(
  DBEngine PRIVATE
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)
