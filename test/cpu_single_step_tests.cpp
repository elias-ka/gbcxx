#include "core/cpu/processor.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <simdjson.h>

namespace cpu = core::cpu;

namespace
{
    constexpr const char* sm83_tests_dir = "3rdparty/sm83-json-tests/v1";
}

class SingleStepParameterizedTest : public testing::TestWithParam<std::filesystem::path>
{
protected:
    static void SetUpTestSuite()
    {
        if (!parser)
            parser = std::make_unique<simdjson::ondemand::parser>();
    }

    void SetUp() override
    {
        std::filesystem::path tests_file = sm83_tests_dir / GetParam();
        json = simdjson::padded_string::load(tests_file.string());
        doc = parser->iterate(json);
    }

    // Parser is shared across tests to avoid needless reallocation
    static std::unique_ptr<simdjson::ondemand::parser> parser;

    simdjson::padded_string json;
    simdjson::ondemand::document doc;
};

std::unique_ptr<simdjson::ondemand::parser> SingleStepParameterizedTest::parser = nullptr;

// Helper to avoid explicitly casting everything in the code below
template <typename T>
constexpr T get(simdjson::ondemand::value value)
{
    return static_cast<T>(value.get_uint64());
}

TEST_P(SingleStepParameterizedTest, SingleStep)
{
    simdjson::ondemand::array root_array = doc.get_array();
    for (simdjson::ondemand::object test : root_array)
    {
        core::memory::mmu mmu{};
        cpu::processor cpu{};
        cpu::registers& reg = cpu.reg();

        simdjson::ondemand::object initial = test["initial"];
        reg.pc = get<uint16_t>(initial["pc"]);
        reg.sp = get<uint16_t>(initial["sp"]);
        reg.a = get<uint8_t>(initial["a"]);
        reg.b = get<uint8_t>(initial["b"]);
        reg.c = get<uint8_t>(initial["c"]);
        reg.d = get<uint8_t>(initial["d"]);
        reg.e = get<uint8_t>(initial["e"]);
        reg.f.raw = get<uint8_t>(initial["f"]);
        reg.h = get<uint8_t>(initial["h"]);
        reg.l = get<uint8_t>(initial["l"]);

        cpu.step(mmu);

        simdjson::ondemand::object final = test["final"];
        EXPECT_EQ(reg.a, get<uint8_t>(final["a"]));
        EXPECT_EQ(reg.b, get<uint8_t>(final["b"]));
        EXPECT_EQ(reg.c, get<uint8_t>(final["c"]));
        EXPECT_EQ(reg.d, get<uint8_t>(final["d"]));
        EXPECT_EQ(reg.e, get<uint8_t>(final["e"]));
        EXPECT_EQ(reg.f.raw, get<uint8_t>(final["f"]));
        EXPECT_EQ(reg.h, get<uint8_t>(final["h"]));
        EXPECT_EQ(reg.l, get<uint8_t>(final["l"]));
        EXPECT_EQ(reg.pc, get<uint16_t>(final["pc"]));
        EXPECT_EQ(reg.sp, get<uint16_t>(final["sp"]));

        // to-do: test ram
    }
}

std::vector<std::filesystem::path> load_test_files()
{
    std::vector<std::filesystem::path> files;
    for (const auto& dirent : std::filesystem::directory_iterator(sm83_tests_dir))
    {
        const auto& path = dirent.path();
        if (path.extension() == ".json")
        {
            files.push_back(path);
        }
    }
    return files;
}

INSTANTIATE_TEST_SUITE_P(JsonFiles, SingleStepParameterizedTest, ::testing::Values("00.json"));
