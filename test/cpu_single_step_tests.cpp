#include "core/emulator.h"
#include "core/processor.h"
#include "core/video.h"
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <simdjson.h>

using namespace cb;

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

struct state
{
    u16 pc, sp;
    cb::cpu::flags f;
    u8 a, b, c, d, e, h, l;

    friend bool operator==(const state& lhs, const state& rhs)
    {
        return lhs.pc == rhs.pc && lhs.sp == rhs.sp && lhs.a == rhs.a && lhs.b == rhs.b &&
               lhs.c == rhs.c && lhs.d == rhs.d && lhs.e == rhs.e && lhs.f.raw == rhs.f.raw &&
               lhs.h == rhs.h && lhs.l == rhs.l;
    }

    friend std::ostream& operator<<(std::ostream& os, const state& state)
    {
        return os << fmt::format("\nPC={:#04x}, SP={:#04x}, A={:#02x}, B={:#02x}, C={:#02x}, "
                                 "D={:#02x}, E={:#02x}, "
                                 "H={:#02x}, L={:#02x}\nFLAGS: {}\n",
                                 state.pc, state.sp, state.a, state.b, state.c, state.d, state.e,
                                 state.h, state.l, state.f);
    }

    friend void PrintTo(const state& state, std::ostream* os) { *os << state; }
};

testing::AssertionResult AssertMyStateEqExpectedState(const state& my_state, const state& expected,
                                                      const state& initial)
{
    if (my_state == expected)
        return testing::AssertionSuccess();

    // to-do: somehow customize the output of GoogleTest
    std::ostringstream oss;
    oss << "Initial state: " << initial << '\n';
    oss << "Expected state: " << expected << '\n';
    oss << "My state: " << my_state << '\n';
    return testing::AssertionFailure() << oss.str();
}

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
        cpu::processor cpu{};
        memory::mmu mmu{};

        simdjson::ondemand::object initial = test["initial"];
        const state initial_state = {.pc = get<u16>(initial["pc"]),
                                     .sp = get<u16>(initial["sp"]),
                                     .f = cb::cpu::flags{get<u8>(initial["f"])},
                                     .a = get<u8>(initial["a"]),
                                     .b = get<u8>(initial["b"]),
                                     .c = get<u8>(initial["c"]),
                                     .d = get<u8>(initial["d"]),
                                     .e = get<u8>(initial["e"]),
                                     .h = get<u8>(initial["h"]),
                                     .l = get<u8>(initial["l"])};

        simdjson::ondemand::object expected = test["final"];
        const state expected_state = {.pc = get<u16>(expected["pc"]),
                                      .sp = get<u16>(expected["sp"]),
                                      .f = cb::cpu::flags{get<u8>(expected["f"])},
                                      .a = get<u8>(expected["a"]),
                                      .b = get<u8>(expected["b"]),
                                      .c = get<u8>(expected["c"]),
                                      .d = get<u8>(expected["d"]),
                                      .e = get<u8>(expected["e"]),
                                      .h = get<u8>(expected["h"]),
                                      .l = get<u8>(expected["l"])};

        cpu::registers& reg = cpu.reg();
        reg.pc = initial_state.pc;
        reg.sp = initial_state.pc;
        reg.a = initial_state.a;
        reg.b = initial_state.b;
        reg.c = initial_state.c;
        reg.d = initial_state.d;
        reg.e = initial_state.e;
        reg.f.raw = initial_state.f.raw;
        reg.h = initial_state.h;
        reg.l = initial_state.l;

        cpu.step(mmu);

        const state my_state = {.pc = reg.pc,
                                .sp = reg.sp,
                                .f = cb::cpu::flags{reg.f.raw},
                                .a = reg.a,
                                .b = reg.b,
                                .c = reg.c,
                                .d = reg.d,
                                .e = reg.e,
                                .h = reg.h,
                                .l = reg.l};

        AssertMyStateEqExpectedState(my_state, expected_state, initial_state);

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
            files.push_back(path.filename());
        }
    }
    return files;
}

INSTANTIATE_TEST_SUITE_P(
    JsonFiles, SingleStepParameterizedTest, ::testing::Values("01.json"),
    [](const testing::TestParamInfo<SingleStepParameterizedTest::ParamType>& info)
    {
        auto stem = info.param.stem().string();
        std::ranges::replace(stem, ' ', '_');
        return stem;
    });
