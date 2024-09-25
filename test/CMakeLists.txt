include(GoogleTest)

add_executable(cringeboy_tests
    cpu_flags_test.cpp
    cpu_registers_test.cpp
    cpu_single_step_tests.cpp
)
target_compile_features(cringeboy_tests PRIVATE cxx_std_20)
target_link_libraries(cringeboy_tests PRIVATE cringeboy_core GTest::gtest_main simdjson::simdjson)

gtest_discover_tests(cringeboy_tests
    PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
