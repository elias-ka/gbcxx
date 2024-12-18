#include "core/constants.hpp"
#include "core/processor.hpp"

// to-do: a lot of duplication in this file can be reduced

namespace gbcxx {
// MARK: 8-bit loads
auto Cpu::ld_r_r(Reg8 dst, Reg8 src) -> void
{
    set_reg(dst, reg(src));
}

auto Cpu::ld_r_n(Reg8 dst) -> void
{
    set_reg(dst, read_operand());
}

auto Cpu::ld_r_mem_hl(Reg8 dst) -> void
{
    set_reg(dst, cycle_read(reg(Reg16::hl)));
}

auto Cpu::ld_mem_hl_r(Reg8 src) -> void
{
    cycle_write(reg(Reg16::hl), reg(src));
}

auto Cpu::ld_mem_hl_n() -> void
{
    cycle_write(reg(Reg16::hl), read_operand());
}

auto Cpu::ld_a_mem_rr(Reg16 src) -> void
{
    m_reg.set(Reg8::a, cycle_read(reg(src)));
}

auto Cpu::ld_mem_rr_a(Reg16 dst) -> void
{
    cycle_write(reg(dst), reg(Reg8::a));
}

auto Cpu::ld_a_mem_nn() -> void
{
    m_reg.set(Reg8::a, cycle_read(read_operands()));
}

auto Cpu::ld_mem_nn_a() -> void
{
    cycle_write(read_operands(), reg(Reg8::a));
}

auto Cpu::ldh_a_mem_c() -> void
{
    set_reg(Reg8::a, cycle_read(0xFF00 + reg(Reg8::c)));
}

auto Cpu::ldh_mem_c_a() -> void
{
    cycle_write(0xFF00 + reg(Reg8::c), reg(Reg8::a));
}

auto Cpu::ldh_a_mem_n() -> void
{
    set_reg(Reg8::a, cycle_read(0xFF00 + read_operand()));
}

auto Cpu::ldh_mem_n_a() -> void
{
    cycle_write(0xFF00 + read_operand(), reg(Reg8::a));
}

auto Cpu::ld_a_mem_hl_dec() -> void
{
    const auto hl = reg(Reg16::hl);
    set_reg(Reg8::a, cycle_read(hl));
    set_reg(Reg16::hl, hl - 1);
}

auto Cpu::ld_mem_hl_dec_a() -> void
{
    const auto hl = reg(Reg16::hl);
    cycle_write(hl, reg(Reg8::a));
    set_reg(Reg16::hl, hl - 1);
}

auto Cpu::ld_a_mem_hl_inc() -> void
{
    const auto hl = reg(Reg16::hl);
    set_reg(Reg8::a, cycle_read(hl));
    set_reg(Reg16::hl, hl + 1);
}

auto Cpu::ld_mem_hl_inc_a() -> void
{
    const auto hl_val = reg(Reg16::hl);
    cycle_write(hl_val, reg(Reg8::a));
    set_reg(Reg16::hl, hl_val + 1);
}

// MARK: 8-bit arithmetic and logical
auto Cpu::add_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint16_t result = reg(Reg8::a) + r_val;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (r_val & 0xF)) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::add_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint16_t result = reg(Reg8::a) + data;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (data & 0xF)) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::add_n() -> void
{
    const uint8_t n = read_operand();
    const uint16_t result = reg(Reg8::a) + n;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (n & 0xF)) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::adc_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint16_t result = reg(Reg8::a) + r_val + flags().c();
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (r_val & 0xF) + flags().c()) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::adc_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint16_t result = reg(Reg8::a) + data + flags().c();
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (data & 0xF) + flags().c()) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::adc_n() -> void
{
    const uint8_t n = read_operand();
    const uint16_t result = reg(Reg8::a) + n + flags().c();
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) + (n & 0xF) + flags().c()) > 0xF);
    set_flag(Flag::c, result > 0xFF);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::sub_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint8_t result = reg(Reg8::a) - r_val;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, true);
    set_flag(Flag::h, (reg(Reg8::a) & 0xF) - (r_val & 0xF) < 0);
    set_flag(Flag::c, reg(Reg8::a) < r_val);
    set_reg(Reg8::a, result);
}

auto Cpu::sub_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint16_t result = reg(Reg8::a) - data;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, true);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) - (data & 0xF)) < 0);
    set_flag(Flag::c, reg(Reg8::a) < data);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::sub_n() -> void
{
    const uint8_t n = read_operand();
    const uint16_t result = reg(Reg8::a) - n;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, true);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) - (n & 0xF)) < 0);
    set_flag(Flag::c, reg(Reg8::a) < n);
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::sbc_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint8_t a = reg(Reg8::a);
    const bool carry = flags().c();
    const int result = a - r_val - carry;
    set_flag(Flag::z, !uint8_t(result));
    set_flag(Flag::n, true);
    set_flag(Flag::h, (a & 0xF) < ((r_val & 0xF) + carry));
    set_flag(Flag::c, a < (r_val + carry));
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::sbc_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint8_t a = reg(Reg8::a);
    const bool carry = flags().c();
    const int result = a - data - carry;
    set_flag(Flag::z, !uint8_t(result));
    set_flag(Flag::n, true);
    set_flag(Flag::h, (a & 0xF) < ((data & 0xF) + carry));
    set_flag(Flag::c, a < (data + carry));
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::sbc_n() -> void
{
    const uint8_t n = read_operand();
    const uint16_t a = reg(Reg8::a);
    const bool carry = flags().c();
    const uint16_t result = a - n - carry;
    set_flag(Flag::z, !uint8_t(result));
    set_flag(Flag::n, true);
    set_flag(Flag::h, ((a & 0xF) < ((n & 0xF) + carry)));
    set_flag(Flag::c, a < (n + carry));
    set_reg(Reg8::a, uint8_t(result));
}

auto Cpu::cp_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint8_t result = reg(Reg8::a) - r_val;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, true);
    set_flag(Flag::h, (reg(Reg8::a) & 0xF) - (r_val & 0xF) < 0);
    set_flag(Flag::c, reg(Reg8::a) < r_val);
}

auto Cpu::cp_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint16_t result = reg(Reg8::a) - data;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, true);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) - (data & 0xF)) < 0);
    set_flag(Flag::c, reg(Reg8::a) < data);
}

auto Cpu::cp_n() -> void
{
    const uint8_t n = read_operand();
    const uint16_t result = reg(Reg8::a) - n;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, true);
    set_flag(Flag::h, ((reg(Reg8::a) & 0xF) - (n & 0xF)) < 0);
    set_flag(Flag::c, reg(Reg8::a) < n);
}

auto Cpu::inc_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint16_t result = r_val + 1;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, (r_val & 0xF) == 0xF);
    set_reg(r, uint8_t(result));
}

auto Cpu::inc_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const uint16_t result = data + 1;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, false);
    set_flag(Flag::h, (data & 0xF) == 0xF);
    cycle_write(addr, uint8_t(result));
}

auto Cpu::dec_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const uint8_t result = r_val - 1;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, true);
    set_flag(Flag::h, (result & 0xF) == 0xF);
    set_reg(r, uint8_t(result));
}

auto Cpu::dec_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const uint16_t result = data - 1;
    set_flag(Flag::z, !(result & 0xFF));
    set_flag(Flag::n, true);
    set_flag(Flag::h, (result & 0xF) == 0xF);
    cycle_write(addr, uint8_t(result));
}

auto Cpu::and_r(Reg8 r) -> void
{
    const uint8_t result = reg(Reg8::a) & reg(r);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, true);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::and_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint8_t result = reg(Reg8::a) & data;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, true);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::and_n() -> void
{
    const uint8_t n = read_operand();
    const uint8_t result = reg(Reg8::a) & n;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, true);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::or_r(Reg8 r) -> void
{
    const uint8_t result = reg(Reg8::a) | reg(r);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::or_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint8_t result = reg(Reg8::a) | data;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::or_n() -> void
{
    const uint8_t n = read_operand();
    const uint8_t result = reg(Reg8::a) | n;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::xor_r(Reg8 r) -> void
{
    const uint8_t result = reg(Reg8::a) ^ reg(r);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::xor_mem_hl() -> void
{
    const uint8_t data = cycle_read(reg(Reg16::hl));
    const uint8_t result = reg(Reg8::a) ^ data;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::xor_n() -> void
{
    const uint8_t n = read_operand();
    const uint8_t result = reg(Reg8::a) ^ n;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(Reg8::a, result);
}

auto Cpu::ccf() -> void
{
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, !flags().c());
}

auto Cpu::scf() -> void
{
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, true);
}

auto Cpu::daa() -> void
{
    // from
    // https://forums.nesdev.org/viewtopic.php?p=196282&sid=38a75719934a07d0ae8ac78a3e1448ad#p196282
    if (!flags().n()) {
        if (flags().c() || reg(Reg8::a) > 0x99) {
            set_reg(Reg8::a, reg(Reg8::a) + 0x60);
            set_flag(Flag::c, true);
        }
        if (flags().h() || (reg(Reg8::a) & 0x0f) > 0x09) {
            set_reg(Reg8::a, reg(Reg8::a) + 0x6);
        }
    }
    else {
        if (flags().c()) {
            set_reg(Reg8::a, reg(Reg8::a) - 0x60);
        }
        if (flags().h()) {
            set_reg(Reg8::a, reg(Reg8::a) - 0x6);
        }
    }

    set_flag(Flag::z, !reg(Reg8::a));
    set_flag(Flag::h, false);
}

auto Cpu::cpl() -> void
{
    set_reg(Reg8::a, ~reg(Reg8::a));
    set_flag(Flag::n, true);
    set_flag(Flag::h, true);
}

// MARK: 16-bit loads
auto Cpu::ld_rr_nn(Reg16 dst) -> void
{
    set_reg(dst, read_operands());
}

auto Cpu::ld_mem_nn_sp() -> void
{
    cycle_write16(read_operands(), reg(Reg16::sp));
}

auto Cpu::ld_sp_hl() -> void
{
    set_reg(Reg16::sp, reg(Reg16::hl));
}

auto Cpu::push_rr(Reg16 src) -> void
{
    push16(reg(src));
}

auto Cpu::pop_rr(Reg16 dst) -> void
{
    set_reg(dst, pop());
}

auto Cpu::ld_hl_sp_e() -> void
{
    const auto e = int8_t(read_operand());
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg16::sp) & 0xF) + (e & 0xF)) > 0xF);
    set_flag(Flag::c, ((reg(Reg16::sp) & 0xFF) + (e & 0xFF)) > 0xFF);
    set_reg(Reg16::hl, reg(Reg16::sp) + uint16_t(e));
}

// MARK: 16-bit arithmetic
auto Cpu::inc_rr(Reg16 rr) -> void
{
    set_reg(rr, reg(rr) + 1);
}

auto Cpu::dec_rr(Reg16 rr) -> void
{
    set_reg(rr, reg(rr) - 1);
}

auto Cpu::add_hl_rr(Reg16 rr) -> void
{
    const uint16_t rr_val = reg(rr);
    const uint16_t hl_val = reg(Reg16::hl);
    const int result = hl_val + rr_val;
    set_flag(Flag::n, false);
    set_flag(Flag::h, (rr_val & 0xFFF) + (hl_val & 0xFFF) > 0xFFF);
    set_flag(Flag::c, result > 0xFFFF);
    set_reg(Reg16::hl, uint16_t(result));
}

auto Cpu::add_sp_e() -> void
{
    const auto e = int8_t(read_operand());
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, ((reg(Reg16::sp) & 0xF) + (e & 0xF)) > 0xF);
    set_flag(Flag::c, ((reg(Reg16::sp) & 0xFF) + (e & 0xFF)) > 0xFF);
    set_reg(Reg16::sp, reg(Reg16::sp) + uint16_t(e));
}

// MARK: Control flow
auto Cpu::jp_nn() -> void
{
    const uint16_t nn = read_operands();
    m_pc = nn;
}

auto Cpu::jp_hl() -> void
{
    m_pc = reg(Reg16::hl);
}

auto Cpu::jp_cc_nn(Condition cc) -> void
{
    const uint16_t nn = read_operands();
    if (check_condition(cc)) {
        m_pc = nn;
        tick4();
    }
}

auto Cpu::jr_e() -> void
{
    const auto e = int8_t(read_operand());
    m_pc += e;
}

auto Cpu::jr_cc_e(Condition cc) -> void
{
    const auto e = int8_t(read_operand());
    if (check_condition(cc)) {
        m_pc += e;
        tick4();
    }
}

auto Cpu::call_nn() -> void
{
    const uint16_t nn = read_operands();
    push16(m_pc);
    m_pc = nn;
}

auto Cpu::call_cc_nn(Condition cc) -> void
{
    const uint16_t nn = read_operands();
    if (check_condition(cc)) {
        push16(m_pc);
        m_pc = nn;
        tick4();
    }
}

auto Cpu::ret() -> void
{
    m_pc = pop();
}

auto Cpu::ret_cc(Condition cc) -> void
{
    if (check_condition(cc)) {
        m_pc = pop();
        tick4();
    }
}

auto Cpu::reti() -> void
{
    m_pc = pop();
    m_ime = true;
}

auto Cpu::rst_n(uint8_t vec) -> void
{
    push16(m_pc);
    m_pc = vec;
}

// MARK: Miscellaneous
auto Cpu::halt() -> void
{
    if (m_ime) {
        m_mode = Mode::halt;
    }
    else {
        const uint8_t intf = m_mmu.read(Io_Reg::if_);
        const uint8_t inte = m_mmu.read(Io_Reg::ie);
        if (inte & intf & 0x1F) {
            m_mode = Mode::halt_bug;
        }
        else {
            m_mode = Mode::halt_di;
        }
    }
}
auto Cpu::stop() -> void
{
    m_mode = Mode::stop;
}
auto Cpu::di() -> void
{
    m_ime = false;
}
auto Cpu::ei() -> void
{
    m_mode = Mode::ime_pending;
}
auto Cpu::nop() -> void { }

// MARK: Rotate, shift, and bit operations
auto Cpu::rlca() -> void
{
    const bool carry = reg(Reg8::a) & (1 << 7);
    const auto result = uint8_t((reg(Reg8::a) << 1) | carry);
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(Reg8::a, result);
}

auto Cpu::rrca() -> void
{
    const bool carry = reg(Reg8::a) & 1;
    const auto result = uint8_t((reg(Reg8::a) >> 1) | (carry << 7));
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(Reg8::a, result);
}

auto Cpu::rla() -> void
{
    const auto result = uint8_t((reg(Reg8::a) << 1) | flags().c());
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, reg(Reg8::a) & (1 << 7));
    set_reg(Reg8::a, result);
}

auto Cpu::rra() -> void
{
    const auto result = uint8_t((reg(Reg8::a) >> 1) | (flags().c() << 7));
    set_flag(Flag::z, false);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, reg(Reg8::a) & 1);
    set_reg(Reg8::a, result);
}

auto Cpu::rlc_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const bool carry = r_val & (1 << 7);
    const auto result = uint8_t((r_val << 1) | carry);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(r, result);
}

auto Cpu::rlc_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const bool carry = data & (1 << 7);
    const auto result = uint8_t((data << 1) | carry);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    cycle_write(addr, result);
}

auto Cpu::rrc_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const bool carry = r_val & 1;
    const auto result = uint8_t((r_val >> 1) | (carry << 7));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(r, result);
}

auto Cpu::rrc_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const bool carry = data & 1;
    const auto result = uint8_t((data >> 1) | (carry << 7));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    cycle_write(addr, result);
}

auto Cpu::rl_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const auto result = uint8_t((r_val << 1) | flags().c());
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, r_val & (1 << 7));
    set_reg(r, result);
}

auto Cpu::rl_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const auto result = uint8_t((data << 1) | flags().c());
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, data & (1 << 7));
    cycle_write(addr, result);
}

auto Cpu::rr_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const auto result = uint8_t((r_val >> 1) | (flags().c() << 7));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, r_val & 1);
    set_reg(r, result);
}

auto Cpu::rr_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const auto result = uint8_t((data >> 1) | (flags().c() << 7));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, data & 1);
    cycle_write(addr, result);
}

auto Cpu::sla_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const bool carry = r_val & (1 << 7);
    const auto result = uint8_t(r_val << 1);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(r, result);
}

auto Cpu::sla_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const bool carry = data & (1 << 7);
    const auto result = uint8_t(data << 1);
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    cycle_write(addr, result);
}

auto Cpu::sra_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const bool carry = r_val & 1;
    const auto result = uint8_t((r_val >> 1) | (r_val & (1 << 7)));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(r, result);
}

auto Cpu::sra_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const bool carry = data & 1;
    const auto result = uint8_t((data >> 1) | (data & (1 << 7)));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    cycle_write(addr, result);
}

auto Cpu::swap_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const auto result = uint8_t(((r_val & 0x0F) << 4) | (((r_val & 0xF0) >> 4) & 0xF));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    set_reg(r, result);
}

auto Cpu::swap_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const auto result = uint8_t(((data & 0x0F) << 4) | (((data & 0xF0) >> 4) & 0xF));
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, false);
    cycle_write(addr, result);
}

auto Cpu::srl_r(Reg8 r) -> void
{
    const uint8_t r_val = reg(r);
    const bool carry = r_val & 1;
    const uint8_t result = r_val >> 1;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    set_reg(r, result);
}

auto Cpu::srl_mem_hl() -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    const bool carry = data & 1;
    const uint8_t result = data >> 1;
    set_flag(Flag::z, !result);
    set_flag(Flag::n, false);
    set_flag(Flag::h, false);
    set_flag(Flag::c, carry);
    cycle_write(addr, result);
}

auto Cpu::bit_b_r(uint8_t b, Reg8 r) -> void
{
    set_flag(Flag::z, !(reg(r) & (1 << b)));
    set_flag(Flag::n, false);
    set_flag(Flag::h, true);
}

auto Cpu::bit_b_mem_hl(uint8_t b) -> void
{
    const auto data = cycle_read(reg(Reg16::hl));
    set_flag(Flag::z, !(data & (1 << b)));
    set_flag(Flag::n, false);
    set_flag(Flag::h, true);
}

auto Cpu::res_b_r(uint8_t b, Reg8 r) -> void
{
    set_reg(r, (reg(r) & ~(1 << b)));
}

auto Cpu::res_b_mem_hl(uint8_t b) -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    cycle_write(addr, data & ~(1 << b));
}

auto Cpu::set_b_r(uint8_t b, Reg8 r) -> void
{
    set_reg(r, uint8_t(reg(r) | (1 << b)));
}

auto Cpu::set_b_mem_hl(uint8_t b) -> void
{
    const uint16_t addr = reg(Reg16::hl);
    const uint8_t data = cycle_read(addr);
    cycle_write(addr, uint8_t(data | (1 << b)));
}

} // namespace gbcxx
