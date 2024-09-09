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

TEST_P(SingleStepParameterizedTest, SingleStep)
{
    simdjson::ondemand::array root_array = doc.get_array();
    for (simdjson::ondemand::object test : root_array)
    {
        core::memory::mmu mmu{};
        cpu::processor cpu{};
        cpu::registers& reg = cpu.reg();

        simdjson::ondemand::object initial = test["initial"];
        reg.pc = static_cast<uint16_t>(initial["pc"].get<uint64_t>());
        reg.sp = static_cast<uint16_t>(initial["sp"].get<uint64_t>());
        reg.a = static_cast<uint8_t>(initial["a"].get<uint64_t>());
        reg.b = static_cast<uint8_t>(initial["b"].get<uint64_t>());
        reg.c = static_cast<uint8_t>(initial["c"].get<uint64_t>());
        reg.d = static_cast<uint8_t>(initial["d"].get<uint64_t>());
        reg.e = static_cast<uint8_t>(initial["e"].get<uint64_t>());
        reg.f.raw = static_cast<uint8_t>(initial["f"].get<uint64_t>());
        reg.h = static_cast<uint8_t>(initial["h"].get<uint64_t>());
        reg.l = static_cast<uint8_t>(initial["l"].get<uint64_t>());

        cpu.step(mmu);

        simdjson::ondemand::object final = test["final"];
        EXPECT_EQ(reg.pc, static_cast<uint16_t>(final["pc"].get<uint64_t>()));
        EXPECT_EQ(reg.sp, static_cast<uint16_t>(final["sp"].get<uint64_t>()));
        EXPECT_EQ(reg.a, static_cast<uint8_t>(final["a"].get<uint64_t>()));
        EXPECT_EQ(reg.b, static_cast<uint8_t>(final["b"].get<uint64_t>()));
        EXPECT_EQ(reg.c, static_cast<uint8_t>(final["c"].get<uint64_t>()));
        EXPECT_EQ(reg.d, static_cast<uint8_t>(final["d"].get<uint64_t>()));
        EXPECT_EQ(reg.e, static_cast<uint8_t>(final["e"].get<uint64_t>()));
        EXPECT_EQ(reg.f.raw, static_cast<uint8_t>(final["f"].get<uint64_t>()));
        EXPECT_EQ(reg.h, static_cast<uint8_t>(final["h"].get<uint64_t>()));
        EXPECT_EQ(reg.l, static_cast<uint8_t>(final["l"].get<uint64_t>()));

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
