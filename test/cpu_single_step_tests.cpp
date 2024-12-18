#include <fmt/format.h>
#include <gtest/gtest.h>
#include <simdjson.h>

#include <filesystem>

#include "core/mbc/mbc0.hpp"
#include "core/memory.hpp"
#include "core/processor.hpp"

using namespace gbcxx;

extern bool g_enable_tracing;

class Single_Step_Parameterized_Test : public testing::TestWithParam<std::filesystem::path> {
public:
    static auto SetUpTestSuite() -> void
    {
        if (g_enable_tracing) {
            spdlog::set_level(spdlog::level::trace);
        }
        if (!parser) {
            parser = simdjson::dom::parser {};
        }
    }

protected:
    static std::optional<simdjson::dom::parser> parser;
};

auto Single_Step_Parameterized_Test::parser = std::optional<simdjson::dom::parser>(std::nullopt);

struct Test_State {
    uint16_t pc, sp;
    Cpu::Flags flags;
    uint8_t a, b, c, d, e, h, l;

    auto operator==(const Test_State&) const -> bool = default;

    friend auto operator<<(std::ostream& os, const Test_State& state) -> std::ostream&
    {
        return os << fmt::format("PC={:#06x} SP={:#06x} A={:#04x} B={:#04x} C={:#04x} "
                                 "D={:#04x} E={:#04x} H={:#04x} L={:#04x} FLAGS: {}",
                                 state.pc, state.sp, state.a, state.b, state.c, state.d, state.e,
                                 state.h, state.l, state.flags);
    }

    friend auto PrintTo(const Test_State& state, std::ostream* os) -> void
    {
        *os << state;
    }
};

auto my_state_eq_expected_state([[maybe_unused]] std::string_view my_state_expr,
                                [[maybe_unused]] std::string_view expected_expr,
                                [[maybe_unused]] std::string_view initial_expr,
                                const Test_State& my_state,
                                const Test_State& expected,
                                const Test_State& initial) -> testing::AssertionResult
{
    if (my_state == expected) {
        return testing::AssertionSuccess();
    }

    // to-do: somehow customize the output of GoogleTest
    std::ostringstream oss;
    oss << "Initial state\t: " << initial << '\n';
    oss << "Expected state\t: " << expected << '\n';
    oss << "My state\t: " << my_state << '\n';
    return testing::AssertionFailure() << oss.str();
}

// Helper to avoid explicitly casting everything in the code below
template <typename T, typename U>
static constexpr auto get_as(U&& value) -> decltype(auto)
{
    return static_cast<T>(std::forward<U>(value).get_uint64());
}

TEST_P(Single_Step_Parameterized_Test, All)
{
    simdjson::dom::array root_array = parser->load(SINGLESTEP_TESTS_DIR / GetParam());
    for (auto test_case : root_array) {
        const auto initial_obj = test_case["initial"].get_object();
        const auto initial = Test_State {
            .pc = get_as<uint16_t>(initial_obj["pc"]),
            .sp = get_as<uint16_t>(initial_obj["sp"]),
            .flags = Cpu::Flags { get_as<uint8_t>(initial_obj["f"]) },
            .a = get_as<uint8_t>(initial_obj["a"]),
            .b = get_as<uint8_t>(initial_obj["b"]),
            .c = get_as<uint8_t>(initial_obj["c"]),
            .d = get_as<uint8_t>(initial_obj["d"]),
            .e = get_as<uint8_t>(initial_obj["e"]),
            .h = get_as<uint8_t>(initial_obj["h"]),
            .l = get_as<uint8_t>(initial_obj["l"]),
        };

        auto mbc = std::make_unique<Mbc0>(std::vector<uint8_t> {});
        auto cpu = Cpu(std::move(mbc));
        cpu.set_pc(initial.pc);
        cpu.set_reg(Cpu::Reg16::sp, initial.sp);
        cpu.set_reg(Cpu::Reg8::a, initial.a);
        cpu.set_reg(Cpu::Reg8::b, initial.b);
        cpu.set_reg(Cpu::Reg8::c, initial.c);
        cpu.set_reg(Cpu::Reg8::d, initial.d);
        cpu.set_reg(Cpu::Reg8::e, initial.e);
        cpu.set_reg(Cpu::Reg8::h, initial.h);
        cpu.set_reg(Cpu::Reg8::l, initial.l);
        cpu.set_flag(Cpu::Flag::z, initial.flags.z());
        cpu.set_flag(Cpu::Flag::n, initial.flags.n());
        cpu.set_flag(Cpu::Flag::h, initial.flags.h());
        cpu.set_flag(Cpu::Flag::c, initial.flags.c());

        cpu.mmu().resize_memory(64_KiB);
        const auto initial_ram = initial_obj["ram"].get_array();
        for (simdjson::dom::array child_arr : initial_ram) {
            const auto addr = get_as<uint16_t>(child_arr.at(0));
            const auto value = get_as<uint8_t>(child_arr.at(1));
            cpu.mmu().write(addr, value);
        }

        cpu.step();

        const auto expected_obj = test_case["final"].get_object();
        const auto expected = Test_State {
            .pc = get_as<uint16_t>(expected_obj["pc"]),
            .sp = get_as<uint16_t>(expected_obj["sp"]),
            .flags = Cpu::Flags { get_as<uint8_t>(expected_obj["f"]) },
            .a = get_as<uint8_t>(expected_obj["a"]),
            .b = get_as<uint8_t>(expected_obj["b"]),
            .c = get_as<uint8_t>(expected_obj["c"]),
            .d = get_as<uint8_t>(expected_obj["d"]),
            .e = get_as<uint8_t>(expected_obj["e"]),
            .h = get_as<uint8_t>(expected_obj["h"]),
            .l = get_as<uint8_t>(expected_obj["l"]),
        };

        const auto expected_ram = expected_obj["ram"].get_array();
        for (simdjson::dom::array child_arr : expected_ram) {
            const auto addr = get_as<uint16_t>(child_arr.at(0));
            const auto value = get_as<uint8_t>(child_arr.at(1));
            EXPECT_EQ(cpu.mmu().read(addr), value);
        }

        const auto my_state = Test_State {
            .pc = cpu.pc(),
            .sp = cpu.sp(),
            .flags = cpu.flags(),
            .a = cpu.reg(Cpu::Reg8::a),
            .b = cpu.reg(Cpu::Reg8::b),
            .c = cpu.reg(Cpu::Reg8::c),
            .d = cpu.reg(Cpu::Reg8::d),
            .e = cpu.reg(Cpu::Reg8::e),
            .h = cpu.reg(Cpu::Reg8::h),
            .l = cpu.reg(Cpu::Reg8::l),
        };

        EXPECT_PRED_FORMAT3(my_state_eq_expected_state, my_state, expected, initial);
    }
}

static auto load_test_files() -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> files;
    for (const auto& dirent : std::filesystem::directory_iterator(SINGLESTEP_TESTS_DIR)) {
        const auto& path = dirent.path();
        if (path.extension() == ".json") {
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

static const std::vector<std::filesystem::path> sixteen_bit_load_instructions
    = { "01.json", "11.json", "21.json", "31.json", "08.json", "f9.json", "c5.json", "d5.json",
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

static const std::vector<std::filesystem::path> sixteen_bit_arithmetic_instructions = {
    "09.json", "19.json", "29.json", "39.json", "e8.json", "03.json", "13.json",
    "23.json", "33.json", "0b.json", "1b.json", "2b.json", "3b.json",
};

static const std::vector<std::filesystem::path> control_flow_instructions = {
    "c3.json", "e9.json", "c2.json", "ca.json", "d2.json", "da.json", "18.json", "20.json",
    "28.json", "30.json", "38.json", "cd.json", "c4.json", "cc.json", "d4.json", "dc.json",
    "c9.json", "c0.json", "c8.json", "d0.json", "d8.json", "d9.json", "c7.json", "cf.json",
    "d7.json", "df.json", "e7.json", "ef.json", "f7.json", "ff.json",
};

static const std::vector<std::filesystem::path> rotate_instructions
    = { "cb 07.json", "cb 0f.json", "cb 17.json", "cb 1f.json", "cb 00.json", "cb 01.json",
        "cb 02.json", "cb 03.json", "cb 04.json", "cb 05.json", "cb 06.json", "cb 08.json",
        "cb 09.json", "cb 0a.json", "cb 0b.json", "cb 0c.json", "cb 0d.json", "cb 0e.json",
        "cb 10.json", "cb 11.json", "cb 12.json", "cb 13.json", "cb 14.json", "cb 15.json",
        "cb 16.json", "cb 1e.json" };

static const std::vector<std::filesystem::path> shift_instructions = {
    "cb 27.json", "cb 20.json", "cb 21.json", "cb 22.json", "cb 23.json", "cb 24.json",
    "cb 25.json", "cb 26.json", "cb 2f.json", "cb 28.json", "cb 29.json", "cb 2a.json",
    "cb 2b.json", "cb 2c.json", "cb 2d.json", "cb 2e.json", "cb 3f.json", "cb 38.json",
    "cb 39.json", "cb 3a.json", "cb 3b.json", "cb 3c.json", "cb 3d.json", "cb 3e.json",
};

static const std::vector<std::filesystem::path> swap_instructions = {
    "cb 37.json", "cb 30.json", "cb 31.json", "cb 32.json",
    "cb 33.json", "cb 34.json", "cb 35.json", "cb 36.json",
};

static const std::vector<std::filesystem::path> bit_instructions = {
    "cb 47.json", "cb 4f.json", "cb 57.json", "cb 5f.json", "cb 67.json", "cb 6f.json",
    "cb 77.json", "cb 7f.json", "cb 40.json", "cb 48.json", "cb 50.json", "cb 58.json",
    "cb 60.json", "cb 68.json", "cb 70.json", "cb 78.json", "cb 41.json", "cb 49.json",
    "cb 51.json", "cb 59.json", "cb 61.json", "cb 69.json", "cb 71.json", "cb 79.json",
    "cb 42.json", "cb 4a.json", "cb 52.json", "cb 5a.json", "cb 62.json", "cb 6a.json",
    "cb 72.json", "cb 7a.json", "cb 43.json", "cb 4b.json", "cb 53.json", "cb 5b.json",
    "cb 63.json", "cb 6b.json", "cb 73.json", "cb 7b.json", "cb 44.json", "cb 4c.json",
    "cb 54.json", "cb 5c.json", "cb 64.json", "cb 6c.json", "cb 74.json", "cb 7c.json",
    "cb 45.json", "cb 4d.json", "cb 55.json", "cb 5d.json", "cb 65.json", "cb 6d.json",
    "cb 75.json", "cb 7d.json", "cb 46.json", "cb 4e.json", "cb 56.json", "cb 5e.json",
    "cb 66.json", "cb 6e.json", "cb 76.json", "cb 7e.json", "cb 87.json", "cb 8f.json",
    "cb 97.json", "cb 9f.json", "cb a7.json", "cb af.json", "cb b7.json", "cb bf.json",
    "cb 80.json", "cb 88.json", "cb 90.json", "cb 98.json", "cb a0.json", "cb a8.json",
    "cb b0.json", "cb b8.json", "cb 81.json", "cb 89.json", "cb 91.json", "cb 99.json",
    "cb a1.json", "cb a9.json", "cb b1.json", "cb b9.json", "cb 82.json", "cb 8a.json",
    "cb 92.json", "cb 9a.json", "cb a2.json", "cb aa.json", "cb b2.json", "cb ba.json",
    "cb 83.json", "cb 8b.json", "cb 93.json", "cb 9b.json", "cb a3.json", "cb ab.json",
    "cb b3.json", "cb bb.json", "cb 84.json", "cb 8c.json", "cb 94.json", "cb 9c.json",
    "cb a4.json", "cb ac.json", "cb b4.json", "cb bc.json", "cb 85.json", "cb 8d.json",
    "cb 95.json", "cb 9d.json", "cb a5.json", "cb ad.json", "cb b5.json", "cb bd.json",
    "cb 86.json", "cb 8e.json", "cb 96.json", "cb 9e.json", "cb a6.json", "cb ae.json",
    "cb b6.json", "cb be.json", "cb c7.json", "cb cf.json", "cb d7.json", "cb df.json",
    "cb e7.json", "cb ef.json", "cb f7.json", "cb ff.json", "cb c0.json", "cb c8.json",
    "cb d0.json", "cb d8.json", "cb e0.json", "cb e8.json", "cb f0.json", "cb f8.json",
    "cb c1.json", "cb c9.json", "cb d1.json", "cb d9.json", "cb e1.json", "cb e9.json",
    "cb f1.json", "cb f9.json", "cb c2.json", "cb ca.json", "cb d2.json", "cb da.json",
    "cb e2.json", "cb ea.json", "cb f2.json", "cb fa.json", "cb c3.json", "cb cb.json",
    "cb d3.json", "cb db.json", "cb e3.json", "cb eb.json", "cb f3.json", "cb fb.json",
    "cb c4.json", "cb cc.json", "cb d4.json", "cb dc.json", "cb e4.json", "cb ec.json",
    "cb f4.json", "cb fc.json", "cb c5.json", "cb cd.json", "cb d5.json", "cb dd.json",
    "cb e5.json", "cb ed.json", "cb f5.json", "cb fd.json", "cb c6.json", "cb ce.json",
    "cb d6.json", "cb de.json", "cb e6.json", "cb ee.json", "cb f6.json", "cb fe.json",
};

INSTANTIATE_TEST_SUITE_P(SingleStepTests,
                         Single_Step_Parameterized_Test,
                         testing::ValuesIn(load_test_files()));
