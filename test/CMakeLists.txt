enable_testing()

add_executable(
  gbcxx_tests EXCLUDE_FROM_ALL main.cpp cpu_flags_test.cpp
                               cpu_registers_test.cpp cpu_single_step_tests.cpp)
target_compile_features(gbcxx_tests PRIVATE cxx_std_20)
target_link_libraries(gbcxx_tests PRIVATE gbcxx_core GTest::gtest_main
                                          simdjson::simdjson)
target_compile_definitions(
  gbcxx_tests
  PRIVATE SINGLESTEP_TESTS_DIR="${CMAKE_SOURCE_DIR}/3rdparty/sm83-json-tests/v1"
)

include(GoogleTest)
gtest_discover_tests(gbcxx_tests)
