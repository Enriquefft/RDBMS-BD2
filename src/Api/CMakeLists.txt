cmake_minimum_required(VERSION 3.26)

add_library(Api Api.cpp)

target_compile_features(Api PUBLIC cxx_std_20)
target_include_directories(Api PUBLIC ${CMAKE_SOURCE_DIR}/include/Api)
target_include_directories(Api PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Api package
find_package(Crow)
target_link_libraries(Api PRIVATE Crow::Crow)
target_link_libraries(Api PUBLIC sql_parser)
