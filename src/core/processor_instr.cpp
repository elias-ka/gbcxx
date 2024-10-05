#include "processor.h"

// to-do: a lot of duplication in this file can be reduced

namespace cb
{
    // 8-bit loads
    void cpu::ld_r_r(reg8 dest, reg8 src) { set_reg(dest, reg(src)); }

    void cpu::ld_r_n(reg8 dest) { set_reg(dest, read_operand()); }

    void cpu::ld_r_mem_hl(reg8 dest) { set_reg(dest, m_mmu->read8(reg(reg16::hl))); }

    void cpu::ld_mem_hl_r(reg8 src) { cycle_write8(reg(reg16::hl), reg(src)); }

    void cpu::ld_mem_hl_n() { cycle_write8(reg(reg16::hl), read_operand()); }

    void cpu::ld_a_mem_rr(reg16 src) { m_a = m_mmu->read8(reg(src)); }

    void cpu::ld_mem_rr_a(reg16 dst) { cycle_write8(reg(dst), m_a); }

    void cpu::ld_a_mem_nn() { m_a = m_mmu->read8(read_operands()); }

    void cpu::ld_mem_nn_a()
    {
        const auto addr = read_operands();
        cycle_write8(addr, m_a);
    }

    void cpu::ldh_a_mem_c()
    {
        const auto addr = static_cast<u16>(0xFF00 + m_c);
        set_reg(reg8::a, m_mmu->read8(addr));
    }

    void cpu::ldh_mem_c_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + m_c);
        cycle_write8(addr, m_a);
    }

    void cpu::ldh_a_mem_n()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        set_reg(reg8::a, m_mmu->read8(addr));
    }

    void cpu::ldh_mem_n_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        cycle_write8(addr, m_a);
    }

    void cpu::ld_a_mem_hl_dec()
    {
        const auto hl = reg(reg16::hl);
        set_reg(reg8::a, m_mmu->read8(hl));
        set_reg(reg16::hl, hl - 1);
    }

    void cpu::ld_mem_hl_dec_a()
    {
        const auto hl = reg(reg16::hl);
        cycle_write8(hl, m_a);
        set_reg(reg16::hl, hl - 1);
    }

    void cpu::ld_a_mem_hl_inc()
    {
        const auto hl = reg(reg16::hl);
        set_reg(reg8::a, m_mmu->read8(hl));
        set_reg(reg16::hl, hl + 1);
    }

    void cpu::ld_mem_hl_inc_a()
    {
        const auto hl_val = reg(reg16::hl);
        cycle_write8(hl_val, m_a);
        set_reg(reg16::hl, hl_val + 1);
    }

    // 8-bit arithmetic and logical
    void cpu::add_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = m_a + r_val;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (r_val & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::add_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a + data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (data & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::add_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a + n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (n & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::adc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = m_a + r_val + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (r_val & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::adc_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a + data + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (data & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::adc_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a + n + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (n & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void cpu::sub_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = m_a - r_val;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (m_a & 0xF) - (r_val & 0xF) < 0);
        m_f.set(flag::c, m_a < r_val);
        m_a = result;
    }

    void cpu::sub_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a - data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF)) < 0);
        m_f.set(flag::c, m_a < data);
        m_a = static_cast<u8>(result);
    }

    void cpu::sub_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a - n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF)) < 0);
        m_f.set(flag::c, m_a < n);
        m_a = static_cast<u8>(result);
    }

    void cpu::sbc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const int result = m_a - r_val - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (r_val & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void cpu::sbc_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const int result = m_a - data - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void cpu::sbc_n()
    {
        const u8 n = read_operand();
        const int result = m_a - n - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void cpu::cp_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = m_a - r_val;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (m_a & 0xF) - (r_val & 0xF) < 0);
        m_f.set(flag::c, m_a < r_val);
    }

    void cpu::cp_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a - data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF)) < 0);
        m_f.set(flag::c, m_a < data);
    }

    void cpu::cp_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a - n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF)) < 0);
        m_f.set(flag::c, m_a < n);
    }

    void cpu::inc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = r_val + 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, (r_val & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void cpu::inc_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const u16 result = data + 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, (data & 0xF) == 0xF);
        cycle_write8(addr, static_cast<u8>(result));
    }

    void cpu::dec_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = r_val - 1;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (result & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void cpu::dec_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const u16 result = data - 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (result & 0xF) == 0xF);
        cycle_write8(addr, static_cast<u8>(result));
    }

    void cpu::and_r(reg8 r)
    {
        const u8 result = m_a & reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::and_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a & data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::and_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a & n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::or_r(reg8 r)
    {
        const u8 result = m_a | reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::or_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a | data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::or_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a | n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::xor_r(reg8 r)
    {
        const u8 result = m_a ^ reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::xor_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a ^ data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::xor_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a ^ n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void cpu::ccf()
    {
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, !m_f.c());
    }

    void cpu::scf()
    {
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, true);
    }

    void cpu::cpl()
    {
        m_a = ~m_a;
        m_f.set(flag::n, true);
        m_f.set(flag::h, true);
    }

    // 16-bit loads
    void cpu::ld_rr_nn(reg16 dst) { set_reg(dst, read_operands()); }

    void cpu::ld_mem_nn_sp() { cycle_write16(read_operands(), m_sp); }

    void cpu::ld_sp_hl() { m_sp = reg(reg16::hl); }

    void cpu::push_rr(reg16 src) { push16(reg(src)); }

    void cpu::pop_rr(reg16 dst) { set_reg(dst, pop16()); }

    void cpu::ld_hl_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_sp & 0xF) + (e & 0xF)) > 0xF);
        m_f.set(flag::c, ((m_sp & 0xFF) + (e & 0xFF)) > 0xFF);
        set_reg(reg16::hl, m_sp + static_cast<u16>(e));
    }

    // 16-bit arithmetic
    void cpu::inc_rr(reg16 rr) { set_reg(rr, reg(rr) + 1); }

    void cpu::dec_rr(reg16 rr) { set_reg(rr, reg(rr) - 1); }

    void cpu::add_hl_rr(reg16 rr)
    {
        const u16 rr_val = reg(rr);
        const u16 hl_val = reg(reg16::hl);
        const int result = hl_val + rr_val;
        m_f.set(flag::n, false);
        m_f.set(flag::h, (rr_val & 0xFFF) + (hl_val & 0xFFF) > 0xFFF);
        m_f.set(flag::c, result > 0xFFFF);
        set_reg(reg16::hl, static_cast<u16>(result));
    }

    void cpu::add_hl_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        const int result = m_sp + e;
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_sp & 0xF) + (e & 0xF)) > 0xF);
        m_f.set(flag::c, ((m_sp & 0xFF) + (e & 0xFF)) > 0xFF);
        m_sp = static_cast<u16>(result);
    }

    // Control flow
    void cpu::jp_nn()
    {
        const u16 nn = read_operands();
        m_pc = nn;
    }

    void cpu::jp_hl() { m_pc = reg(reg16::hl); }

    void cpu::jp_cc_nn(condition cc)
    {
        const u16 nn = read_operands();
        if (check_condition(cc))
        {
            m_pc = nn;
        }
    }

    void cpu::jr_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_pc += e;
    }

    void cpu::jr_cc_e(condition cc)
    {
        const auto e = static_cast<s8>(read_operand());
        if (check_condition(cc))
        {
            m_pc += e;
        }
    }

    void cpu::call_nn()
    {
        const u16 nn = read_operands();
        push16(m_pc);
        m_pc = nn;
    }

    void cpu::call_cc_nn(condition cc)
    {
        const u16 nn = read_operands();
        if (check_condition(cc))
        {
            push16(m_pc);
            m_pc = nn;
        }
    }

    void cpu::ret() { m_pc = pop16(); }

    void cpu::ret_cc(condition cc)
    {
        if (check_condition(cc))
        {
            m_pc = pop16();
        }
    }

    void cpu::reti()
    {
        m_pc = pop16();
        m_ime = true;
    }

    void cpu::rst_n(u8 vec)
    {
        push16(m_pc);
        m_pc = vec;
    }

    // Miscellaneous
    void cpu::nop() {}

    // Rotate, shift, and bit operations
    void cpu::rlca()
    {
        const bool carry = m_a & BIT(7);
        const auto result = static_cast<u8>((m_a << 1) | carry);
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        m_a = result;
    }

    void cpu::rrca()
    {
        const bool carry = m_a & BIT(0);
        const auto result = static_cast<u8>((m_a >> 1) | (carry << 7));
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        m_a = result;
    }

    void cpu::rla()
    {
        const auto result = static_cast<u8>((m_a << 1) | m_f.c());
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, m_a & BIT(7));
        m_a = result;
    }

    void cpu::rra()
    {
        const auto result = static_cast<u8>((m_a >> 1) | (m_f.c() << 7));
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, m_a & BIT(0));
        m_a = result;
    }

    void cpu::rlc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(7);
        const auto result = static_cast<u8>((r_val << 1) | carry);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        set_reg(r, result);
    }

    void cpu::rlc_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const bool carry = data & BIT(7);
        const auto result = static_cast<u8>((data << 1) | carry);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        cycle_write8(addr, result);
    }

    void cpu::rrc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(0);
        const auto result = static_cast<u8>((r_val >> 1) | (carry << 7));
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        set_reg(r, result);
    }

    void cpu::rrc_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const bool carry = data & BIT(0);
        const auto result = static_cast<u8>((data >> 1) | (carry << 7));
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, carry);
        cycle_write8(addr, result);
    }

    void cpu::rl_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const auto result = static_cast<u8>((r_val << 1) | m_f.c());
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, r_val & BIT(7));
        set_reg(r, result);
    }

    void cpu::rl_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const auto result = static_cast<u8>((data << 1) | m_f.c());
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, data & BIT(7));
        cycle_write8(addr, result);
    }

    void cpu::rr_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const auto result = static_cast<u8>((r_val >> 1) | (m_f.c() << 7));
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, r_val & BIT(0));
        set_reg(r, result);
    }

    void cpu::rr_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const auto result = static_cast<u8>((data >> 1) | (m_f.c() << 7));
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, data & BIT(0));
        cycle_write8(addr, result);
    }

} // namespace cb
