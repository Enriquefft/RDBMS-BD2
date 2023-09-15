add_library(Api Api.cpp)

target_include_directories(Api PUBLIC ${CMAKE_SOURCE_DIR}/include/Api)

# Api package
find_package(Crow)
target_link_libraries(Api PRIVATE Crow::Crow)
