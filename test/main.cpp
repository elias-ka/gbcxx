#include <gtest/gtest.h>

bool g_enable_tracing = false;

auto main(int argc, char** argv) -> int
{
    using namespace std::string_view_literals;

    if (argc > 2 and std::string_view(argv[2]) == "--trace"sv) {
        g_enable_tracing = true;
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
