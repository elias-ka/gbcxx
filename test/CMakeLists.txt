add_executable(gbcxx_tests main.cpp cpu_registers_test.cpp
                           cpu_single_step_tests.cpp)
target_compile_features(gbcxx_tests PRIVATE cxx_std_23)
target_link_libraries(gbcxx_tests PRIVATE gbcxx_core GTest::gtest_main
                                          simdjson::simdjson)
target_compile_definitions(
  gbcxx_tests
  PRIVATE SINGLESTEP_TESTS_DIR="${CMAKE_SOURCE_DIR}/3rdparty/sm83-json-tests/v1"
)

include(GoogleTest)
gtest_discover_tests(gbcxx_tests)
