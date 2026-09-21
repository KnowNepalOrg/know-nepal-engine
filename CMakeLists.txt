cmake_minimum_required(VERSION 3.20)
project(know-nepal-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(engine_lib STATIC
    src/normalize.cpp
    src/id_generator.cpp
    src/index_builder.cpp
    src/levenshtein.cpp
    src/search.cpp
)
target_include_directories(engine_lib PUBLIC include)
target_link_libraries(engine_lib PUBLIC nlohmann_json::nlohmann_json)

add_executable(index-builder src/main.cpp)
target_link_libraries(index-builder PRIVATE engine_lib)

add_executable(search-engine src/search_main.cpp)
target_link_libraries(search-engine PRIVATE engine_lib)

add_executable(bench-cpp benchmark/bench-cpp.cpp)
target_link_libraries(bench-cpp PRIVATE engine_lib)

enable_testing()
add_subdirectory(tests)
