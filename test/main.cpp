#include <gtest/gtest.h>

// NOLINTNEXTLINE(misc-use-internal-linkage)
bool g_enable_tracing = false;

int main(int argc, char** argv)
{
    using namespace std::string_view_literals;

    if (argc > 2 && std::string_view(argv[2]) == "--trace"sv)
    {
        g_enable_tracing = true;
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
