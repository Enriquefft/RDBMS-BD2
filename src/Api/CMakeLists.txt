cmake_minimum_required(VERSION 3.26)

add_library(Api Api.cpp)

target_include_directories(Api PUBLIC ${CMAKE_SOURCE_DIR}/include/Api)

target_compile_features(Api PRIVATE cxx_std_20)
target_compile_options(
  Api
  PRIVATE # Prefered warnings
          $<$<CXX_COMPILER_ID:MSVC>:/W4
          /WX>
          $<$<CXX_COMPILER_ID:Clang>:-Weverything>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
          # -Werror
          -Wall
          -Wextra
          -Wpedantic
          -Wformat=2
          -Wno-unused-parameter
          -Wshadow
          -Wwrite-strings
          -Wcast-qual
          -Wcast-align
          -Wconversion> # Disable some warnings when using clang's Weverything
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
  Api PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

# Api package
find_package(Crow)
target_link_libraries(Api PRIVATE Crow::Crow)
