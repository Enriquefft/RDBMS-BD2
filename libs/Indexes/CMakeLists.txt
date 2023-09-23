cmake_minimum_required(VERSION 3.16)
project(Indexes)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_library(Indexes INTERFACE)

target_compile_features(Indexes INTERFACE cxx_std_20)
target_include_directories(Indexes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# target_sources(Indexes INTERFACE index.tpp AVL/avl_index.tpp)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  add_executable(IndexesTest test/test.cpp)

  target_compile_features(IndexesTest PRIVATE cxx_std_20)

  target_link_libraries(IndexesTest PRIVATE Indexes)

endif()
