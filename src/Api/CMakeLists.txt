add_library(Api Api.cpp)

target_include_directories(Api PUBLIC ${CMAKE_SOURCE_DIR}/include/Api)

target_include_directories(Api SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/libs/Crow)
