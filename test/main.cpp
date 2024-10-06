#include <gtest/gtest.h>

bool g_enable_tracing = false;

int main(int argc, char** argv)
{
    if (argc > 2 && std::string_view{argv[2]} == "--trace")
    {
        g_enable_tracing = true;
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
