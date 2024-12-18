#pragma once

#include <fmt/base.h>

#include <array>
#include <string>

#include "core/mbc.hpp"
#include "core/memory.hpp"
#include "util.hpp"

namespace gbcxx {

class Cpu {
public:
    enum class Reg8 { a, b, c, d, e, h, l };
    enum class Reg16 { af, bc, de, hl, sp };
    enum class Condition { nz, z, nc, c };
    enum class Flag { z, n, h, c };

    struct Flags {
        uint8_t raw;

        auto operator==(const Flags&) const -> bool = default;

        [[nodiscard]] auto z() const -> bool;
        [[nodiscard]] auto n() const -> bool;
        [[nodiscard]] auto h() const -> bool;
        [[nodiscard]] auto c() const -> bool;

        auto set(Flag f, bool set = true) -> void;
    };

    class Registers {
    public:
        static auto make_dmg() -> Registers
        {
            Registers r;
            r.m_a = 0x01;
            r.m_b = 0x00;
            r.m_c = 0x13;
            r.m_d = 0x00;
            r.m_e = 0xD8;
            r.m_h = 0x01;
            r.m_l = 0x4D;
            r.m_sp = 0xFFFE;
            r.m_f = Flags { 0xB0 };
            return r;
        }

        [[nodiscard]] auto flags(this auto&& self) -> auto&&
        {
            return FWD(self).m_f;
        }

        [[nodiscard]] auto get(Reg8 reg) const -> uint8_t;
        [[nodiscard]] auto get(Reg16 reg) const -> uint16_t;

        auto set(Reg8 reg, uint8_t val) -> void;
        auto set(Reg16 reg, uint16_t val) -> void;

    private:
        uint8_t m_a {};
        uint8_t m_b {};
        uint8_t m_c {};
        uint8_t m_d {};
        uint8_t m_e {};
        uint8_t m_h {};
        uint8_t m_l {};
        uint16_t m_sp {};
        Flags m_f {};
    };

    explicit Cpu(std::unique_ptr<Mbc> mbc)
        : m_mmu(std::move(mbc))
    {
    }

    auto step() -> void;

    auto subtract_cycles(size_t cycles) -> void
    {
        m_cycles_elapsed -= cycles;
    }

    [[nodiscard]] auto cycles_elapsed() const -> size_t
    {
        return m_cycles_elapsed;
    }

    [[nodiscard]] auto pc() const -> uint16_t
    {
        return m_pc;
    }

    auto set_pc(uint16_t pc) -> void
    {
        m_pc = pc;
    }

    [[nodiscard]] auto sp() const -> uint16_t
    {
        return m_reg.get(Reg16::sp);
    }

    [[nodiscard]] auto flags() const -> Flags
    {
        return m_reg.flags();
    }
    auto set_flag(Flag f, bool set = true) -> void
    {
        m_reg.flags().set(f, set);
    }

    [[nodiscard]] auto reg(Reg8 reg) const -> uint8_t
    {
        return m_reg.get(reg);
    }

    [[nodiscard]] auto reg(Reg16 reg) const -> uint16_t
    {
        return m_reg.get(reg);
    }

    auto set_reg(Reg8 reg, uint8_t val) -> void
    {
        m_reg.set(reg, val);
    }

    auto set_reg(Reg16 reg, uint16_t val) -> void
    {
        m_reg.set(reg, val);
    }

    auto mmu() -> Mmu&
    {
        return m_mmu;
    }

private:
    auto log_for_gameboy_doctor() -> void;

    auto tick() -> void;
    auto tick4() -> void;

    auto cycle_read(uint16_t addr) -> uint8_t;
    auto cycle_read16(uint16_t addr) -> uint16_t;
    auto cycle_write(uint16_t addr, uint8_t data) -> void;
    auto cycle_write16(uint16_t addr, uint16_t data) -> void;

    auto read_operand() -> uint8_t;
    auto read_operands() -> uint16_t;

    auto push(uint8_t value) -> void;
    auto push16(uint16_t value) -> void;
    auto pop() -> uint16_t;

    auto handle_interrupt() -> void;
    auto interpret_instruction(uint8_t opcode) -> void;

    [[nodiscard]] auto check_condition(Condition cond) const -> bool
    {
        const auto flags = m_reg.flags();
        switch (cond) {
        case Condition::nz: return !flags.z();
        case Condition::z: return flags.z();
        case Condition::nc: return !flags.c();
        case Condition::c: return flags.c();
        }
    }

private:
    enum class Mode { default_, halt, stop, halt_bug, halt_di, ime_pending };

    Mmu m_mmu;
    Registers m_reg { Registers::make_dmg() };
    size_t m_cycles_elapsed {};
    uint16_t m_pc { 0x100 };
    Mode m_mode { Mode::default_ };
    bool m_ime { true };
    std::fstream m_log_file { "log.txt", std::ios::out };

private:
    // 8-bit loads
    auto ld_r_r(Reg8 dst, Reg8 src) -> void;
    auto ld_r_n(Reg8 dst) -> void;
    auto ld_r_mem_hl(Reg8 dst) -> void;
    auto ld_mem_hl_r(Reg8 src) -> void;
    auto ld_mem_hl_n() -> void;
    auto ld_a_mem_rr(Reg16 src) -> void;
    auto ld_mem_rr_a(Reg16 dst) -> void;
    auto ld_a_mem_nn() -> void;
    auto ld_mem_nn_a() -> void;
    auto ldh_a_mem_c() -> void;
    auto ldh_mem_c_a() -> void;
    auto ldh_a_mem_n() -> void;
    auto ldh_mem_n_a() -> void;
    auto ld_a_mem_hl_dec() -> void;
    auto ld_mem_hl_dec_a() -> void;
    auto ld_a_mem_hl_inc() -> void;
    auto ld_mem_hl_inc_a() -> void;

    // 8-bit arithmetic and logical
    auto add_r(Reg8 r) -> void;
    auto add_mem_hl() -> void;
    auto add_n() -> void;
    auto adc_r(Reg8 r) -> void;
    auto adc_mem_hl() -> void;
    auto adc_n() -> void;
    auto sub_r(Reg8 r) -> void;
    auto sub_mem_hl() -> void;
    auto sub_n() -> void;
    auto sbc_r(Reg8 r) -> void;
    auto sbc_mem_hl() -> void;
    auto sbc_n() -> void;
    auto cp_r(Reg8 r) -> void;
    auto cp_mem_hl() -> void;
    auto cp_n() -> void;
    auto inc_r(Reg8 r) -> void;
    auto inc_mem_hl() -> void;
    auto dec_r(Reg8 r) -> void;
    auto dec_mem_hl() -> void;
    auto and_r(Reg8 r) -> void;
    auto and_mem_hl() -> void;
    auto and_n() -> void;
    auto or_r(Reg8 r) -> void;
    auto or_mem_hl() -> void;
    auto or_n() -> void;
    auto xor_r(Reg8 r) -> void;
    auto xor_mem_hl() -> void;
    auto xor_n() -> void;
    auto ccf() -> void;
    auto scf() -> void;
    auto daa() -> void;
    auto cpl() -> void;

    // 16-bit loads
    auto ld_rr_nn(Reg16 dst) -> void;
    auto ld_mem_nn_sp() -> void;
    auto ld_sp_hl() -> void;
    auto push_rr(Reg16 src) -> void;
    auto pop_rr(Reg16 dst) -> void;
    auto ld_hl_sp_e() -> void;

    // 16-bit arithmetic
    auto inc_rr(Reg16 rr) -> void;
    auto dec_rr(Reg16 rr) -> void;
    auto add_hl_rr(Reg16 rr) -> void;
    auto add_sp_e() -> void;

    // Control flow
    auto jp_nn() -> void;
    auto jp_hl() -> void;
    auto jp_cc_nn(Condition cc) -> void;
    auto jr_e() -> void;
    auto jr_cc_e(Condition cc) -> void;
    auto call_nn() -> void;
    auto call_cc_nn(Condition cc) -> void;
    auto ret() -> void;
    auto ret_cc(Condition cc) -> void;
    auto reti() -> void;
    auto rst_n(uint8_t vec) -> void;

    // Miscellaneous
    auto halt() -> void;
    auto stop() -> void;
    auto di() -> void;
    auto ei() -> void;
    auto nop() -> void;

    // Rotate, shift, and bit operations
    auto rlca() -> void;
    auto rrca() -> void;
    auto rla() -> void;
    auto rra() -> void;
    auto rlc_r(Reg8 r) -> void;
    auto rlc_mem_hl() -> void;
    auto rrc_r(Reg8 r) -> void;
    auto rrc_mem_hl() -> void;
    auto rl_r(Reg8 r) -> void;
    auto rl_mem_hl() -> void;
    auto rr_r(Reg8 r) -> void;
    auto rr_mem_hl() -> void;
    auto sla_r(Reg8 r) -> void;
    auto sla_mem_hl() -> void;
    auto sra_r(Reg8 r) -> void;
    auto sra_mem_hl() -> void;
    auto swap_r(Reg8 r) -> void;
    auto swap_mem_hl() -> void;
    auto srl_r(Reg8 r) -> void;
    auto srl_mem_hl() -> void;
    auto bit_b_r(uint8_t b, Reg8 r) -> void;
    auto bit_b_mem_hl(uint8_t b) -> void;
    auto res_b_r(uint8_t b, Reg8 r) -> void;
    auto res_b_mem_hl(uint8_t b) -> void;
    auto set_b_r(uint8_t b, Reg8 r) -> void;
    auto set_b_mem_hl(uint8_t b) -> void;
};

inline auto format_as(Cpu::Flags f) -> std::string
{
    const std::array<char, 4> symbols
        = { f.z() ? 'Z' : '-', f.n() ? 'N' : '-', f.h() ? 'H' : '-', f.c() ? 'C' : '-' };
    return { symbols.begin(), symbols.end() };
}

} // namespace gbcxx
