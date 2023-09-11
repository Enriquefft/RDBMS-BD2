add_library(DBEngine DBEngine.cpp)
target_sources(DBEngine PRIVATE HeapFile.cpp Record/Record.cpp)

target_compile_features(DBEngine PRIVATE cxx_std_20)
target_include_directories(DBEngine PUBLIC ${CMAKE_SOURCE_DIR}/include/DBEngine)
