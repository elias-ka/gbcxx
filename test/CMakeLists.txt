include(GoogleTest)

function(add_gtest test_name test_sources)
    add_executable(${test_name} ${test_sources})
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_link_libraries(${test_name} PRIVATE cringeboy_base GTest::gtest_main)

    gtest_discover_tests(${test_name})
endfunction()


add_gtest(cpu_registers_test cpu_registers_test.cpp)
add_gtest(cpu_flags_test cpu_flags_test.cpp)
