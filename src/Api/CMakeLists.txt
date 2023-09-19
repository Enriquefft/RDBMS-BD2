add_library(Api Api.cpp)

target_include_directories(Api PUBLIC ${CMAKE_SOURCE_DIR}/include/Api)

# Api package
find_package(Crow QUIET REQUIRED)

if(Crow_FOUND)
  message(STATUS "Found Crow")
  target_link_libraries(Api PRIVATE Crow::Crow)
else()
  message(STATUS "Crow not found, using bundled version")
  target_include_directories(Api SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/libs/Crow)
endif()
