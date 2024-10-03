#include "core/processor.h"
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <simdjson.h>

using namespace cb;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern bool g_enable_tracing;

class SingleStepParameterizedTest : public testing::TestWithParam<std::filesystem::path>
{
public:
    static void SetUpTestSuite()
    {
        if (g_enable_tracing)
        {
            logger::instance().set_log_level(log_level::trace);
        }
        if (!parser)
            parser = std::make_unique<simdjson::dom::parser>();
    }

protected:
    // Parser is shared across tests to avoid needless reallocation
    static std::unique_ptr<simdjson::dom::parser> parser;
};

std::unique_ptr<simdjson::dom::parser> SingleStepParameterizedTest::parser = nullptr;

struct test_state
{
    u16 pc, sp;
    flags f;
    u8 a, b, c, d, e, h, l;

    bool operator==(const test_state&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const test_state& state)
    {
        return os << fmt::format("PC={:#06x} SP={:#06x} A={:#04x} B={:#04x} C={:#04x} "
                                 "D={:#04x} E={:#04x} H={:#04x} L={:#04x} FLAGS: {}",
                                 state.pc, state.sp, state.a, state.b, state.c, state.d, state.e,
                                 state.h, state.l, state.f);
    }

    friend void PrintTo(const test_state& state, std::ostream* os) { *os << state; }
};

testing::AssertionResult MyStateEqExpectedState([[maybe_unused]] const char* my_state_expr,
                                                [[maybe_unused]] const char* expected_expr,
                                                [[maybe_unused]] const char* initial_expr,
                                                const test_state& my_state,
                                                const test_state& expected,
                                                const test_state& initial)
{
    if (my_state == expected)
        return testing::AssertionSuccess();

    // to-do: somehow customize the output of GoogleTest
    std::ostringstream oss;
    oss << "Initial state\t: " << initial << '\n';
    oss << "Expected state\t: " << expected << '\n';
    oss << "My state\t: " << my_state << '\n';
    return testing::AssertionFailure() << oss.str();
}

// Helper to avoid explicitly casting everything in the code below
template <typename T, typename U>
static constexpr decltype(auto) get(U&& value)
{
    return static_cast<T>(std::forward<U>(value).get_uint64());
}

TEST_P(SingleStepParameterizedTest, All)
{
    simdjson::dom::array root_array = parser->load(SINGLESTEP_TESTS_DIR / GetParam());
    for (auto test : root_array)
    {
        auto initial_obj = test["initial"].get_object();
        const test_state initial = {.pc = get<u16>(initial_obj["pc"]),
                                    .sp = get<u16>(initial_obj["sp"]),
                                    .f = flags{get<u8>(initial_obj["f"])},
                                    .a = get<u8>(initial_obj["a"]),
                                    .b = get<u8>(initial_obj["b"]),
                                    .c = get<u8>(initial_obj["c"]),
                                    .d = get<u8>(initial_obj["d"]),
                                    .e = get<u8>(initial_obj["e"]),
                                    .h = get<u8>(initial_obj["h"]),
                                    .l = get<u8>(initial_obj["l"])};

        mmu mmu{cartridge{mbc_rom_only{{}}}};
        mmu.resize_memory(64_KiB);

        auto ram = initial_obj["ram"].get_array();
        for (simdjson::dom::array child_arr : ram)
        {
            const auto addr = get<u16>(child_arr.at(0));
            const auto value = get<u8>(child_arr.at(1));
            mmu.write8(addr, value);
        }

        processor cpu{&mmu,      initial.pc, initial.sp, initial.f, initial.a, initial.b,
                      initial.c, initial.d,  initial.e,  initial.h, initial.l};

        cpu.step();

        auto expected_obj = test["final"].get_object();
        const test_state expected = {.pc = get<u16>(expected_obj["pc"]),
                                     .sp = get<u16>(expected_obj["sp"]),
                                     .f = flags{get<u8>(expected_obj["f"])},
                                     .a = get<u8>(expected_obj["a"]),
                                     .b = get<u8>(expected_obj["b"]),
                                     .c = get<u8>(expected_obj["c"]),
                                     .d = get<u8>(expected_obj["d"]),
                                     .e = get<u8>(expected_obj["e"]),
                                     .h = get<u8>(expected_obj["h"]),
                                     .l = get<u8>(expected_obj["l"])};

        const test_state my_state = {.pc = cpu.pc(),
                                     .sp = cpu.sp(),
                                     .f = cpu.f(),
                                     .a = cpu.reg(reg8::a),
                                     .b = cpu.reg(reg8::b),
                                     .c = cpu.reg(reg8::c),
                                     .d = cpu.reg(reg8::d),
                                     .e = cpu.reg(reg8::e),
                                     .h = cpu.reg(reg8::h),
                                     .l = cpu.reg(reg8::l)};

        EXPECT_PRED_FORMAT3(MyStateEqExpectedState, my_state, expected, initial);
    }
}

static std::vector<std::filesystem::path> load_test_files()
{
    std::vector<std::filesystem::path> files;
    for (const auto& dirent : std::filesystem::directory_iterator(SINGLESTEP_TESTS_DIR))
    {
        const auto& path = dirent.path();
        if (path.extension() == ".json")
        {
            files.push_back(path.filename());
        }
    }
    return files;
}

static const std::vector<std::filesystem::path> eight_bit_load_instructions = {
    "7f.json", "78.json", "79.json", "7a.json", "7b.json", "7c.json", "7d.json", "47.json",
    "41.json", "42.json", "43.json", "44.json", "45.json", "4f.json", "48.json", "49.json",
    "4a.json", "4b.json", "4c.json", "4d.json", "57.json", "50.json", "51.json", "52.json",
    "53.json", "54.json", "55.json", "5f.json", "58.json", "59.json", "5a.json", "5b.json",
    "5c.json", "5d.json", "67.json", "60.json", "61.json", "62.json", "63.json", "64.json",
    "65.json", "6f.json", "68.json", "69.json", "6a.json", "6b.json", "6c.json", "6d.json",
    "3e.json", "06.json", "0e.json", "16.json", "1e.json", "26.json", "2e.json", "7e.json",
    "46.json", "4e.json", "56.json", "5e.json", "66.json", "6e.json", "77.json", "70.json",
    "71.json", "72.json", "73.json", "74.json", "75.json", "36.json", "0a.json", "1a.json",
    "02.json", "12.json", "fa.json", "ea.json", "f2.json", "e2.json", "f0.json", "e0.json",
    "3a.json", "32.json", "2a.json", "22.json",
};

static const std::vector<std::filesystem::path> sixteen_bit_load_instructions = {
    "01.json", "11.json", "21.json", "31.json", "08.json", "f9.json", "c5.json", "d5.json",
    "e5.json", "f5.json", "c1.json", "d1.json", "e1.json", "f1.json", "f8.json"

};

static const std::vector<std::filesystem::path> eight_bit_arithmetic_and_logical_instructions = {
    "af.json", "a8.json", "a9.json", "aa.json", "ab.json", "ac.json", "ad.json", "87.json",
    "80.json", "81.json", "82.json", "83.json", "84.json", "85.json", "86.json", "c6.json",
    "8f.json", "88.json", "89.json", "8a.json", "8b.json", "8c.json", "8d.json", "8e.json",
    "ce.json", "97.json", "90.json", "91.json", "92.json", "93.json", "94.json", "95.json",
    "96.json", "d6.json", "9f.json", "98.json", "99.json", "9a.json", "9b.json", "9c.json",
    "9d.json", "9e.json", "de.json", "bf.json", "b8.json", "b9.json", "ba.json", "bb.json",
    "bc.json", "bd.json", "be.json", "fe.json", "3c.json", "04.json", "0c.json", "14.json",
    "1c.json", "24.json", "2c.json", "34.json", "3d.json", "05.json", "0d.json", "15.json",
    "1d.json", "25.json", "2d.json", "35.json", "a7.json", "a0.json", "a1.json", "a2.json",
    "a3.json", "a4.json", "a5.json", "a6.json", "e6.json", "b7.json", "b0.json", "b1.json",
    "b2.json", "b3.json", "b4.json", "b5.json", "b6.json", "f6.json", "ae.json", "ee.json",
    "3f.json", "37.json", "2f.json",
};

INSTANTIATE_TEST_SUITE_P(SingleStepTests, SingleStepParameterizedTest,
                         testing::ValuesIn(eight_bit_arithmetic_and_logical_instructions));
