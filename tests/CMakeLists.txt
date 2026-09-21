find_package(GTest CONFIG REQUIRED)

add_executable(engine_tests
    test_normalize.cpp
    test_id_generator.cpp
    test_index_builder.cpp
    test_levenshtein.cpp
    test_search.cpp
)
target_link_libraries(engine_tests PRIVATE engine_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(engine_tests)
