#include "processor.h"

// to-do: a lot of duplication in this file can be reduced

namespace cb
{
    // 8-bit loads
    void processor::ld_r_r(reg8 dest, reg8 src) { set_reg(dest, reg(src)); }

    void processor::ld_r_n(reg8 dest) { set_reg(dest, read_operand()); }

    void processor::ld_r_mem_hl(reg8 dest) { set_reg(dest, m_mmu->read8(reg(reg16::hl))); }

    void processor::ld_mem_hl_r(reg8 src) { write8(m_mmu->read8(reg(reg16::hl)), reg(src)); }

    void processor::ld_mem_hl_n() { write8(m_mmu->read8(reg(reg16::hl)), read_operand()); }

    void processor::ld_a_mem_rr(reg16 src) { m_a = m_mmu->read8(reg(src)); }

    void processor::ld_mem_rr_a(reg16 dst) { write8(m_mmu->read8(reg(dst)), m_a); }

    void processor::ld_a_mem_nn() { m_a = m_mmu->read8(read_operands()); }

    void processor::ld_mem_nn_a()
    {
        const auto addr = read_operands();
        write8(addr, m_a);
    }

    void processor::ldh_a_mem_c()
    {
        const auto addr = static_cast<u16>(0xFF00 + m_c);
        set_reg(reg8::a, m_mmu->read8(addr));
    }

    void processor::ldh_mem_c_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + m_c);
        write8(addr, m_a);
    }

    void processor::ldh_a_mem_n()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        set_reg(reg8::a, m_mmu->read8(addr));
    }

    void processor::ldh_mem_n_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        write8(addr, m_a);
    }

    void processor::ld_a_mem_hl_dec()
    {
        const auto hl = reg(reg16::hl);
        set_reg(reg8::a, m_mmu->read8(hl));
        set_reg(reg16::hl, hl - 1);
    }

    void processor::ld_mem_hl_dec_a()
    {
        const auto hl = reg(reg16::hl);
        write8(hl, m_a);
        set_reg(reg16::hl, hl - 1);
    }

    void processor::ld_a_mem_hl_inc()
    {
        const auto hl = reg(reg16::hl);
        set_reg(reg8::a, m_mmu->read8(hl));
        set_reg(reg16::hl, hl + 1);
    }

    void processor::ld_mem_hl_inc_a()
    {
        const auto hl = reg(reg16::hl);
        write8(m_mmu->read8(hl), m_a);
        set_reg(reg16::hl, hl + 1);
    }

    // 8-bit arithmetic and logical
    void processor::add_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = m_a + r_val;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (r_val & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::add_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a + data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (data & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::add_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a + n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (n & 0xF)) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::adc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = m_a + r_val + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (r_val & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::adc_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a + data + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (data & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::adc_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a + n + m_f.c();
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_a & 0xF) + (n & 0xF) + m_f.c()) > 0xF);
        m_f.set(flag::c, result > 0xFF);
        m_a = static_cast<u8>(result);
    }

    void processor::sub_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = m_a - r_val;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (m_a & 0xF) - (r_val & 0xF) < 0);
        m_f.set(flag::c, m_a < r_val);
        m_a = result;
    }

    void processor::sub_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a - data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF)) < 0);
        m_f.set(flag::c, m_a < data);
        m_a = static_cast<u8>(result);
    }

    void processor::sub_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a - n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF)) < 0);
        m_f.set(flag::c, m_a < n);
        m_a = static_cast<u8>(result);
    }

    void processor::sbc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const int result = m_a - r_val - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (r_val & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void processor::sbc_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const int result = m_a - data - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void processor::sbc_n()
    {
        const u8 n = read_operand();
        const int result = m_a - n - m_f.c();
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF) - m_f.c()) < 0);
        m_f.set(flag::c, result < 0);
        m_a = static_cast<u8>(result);
    }

    void processor::cp_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = m_a - r_val;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (m_a & 0xF) - (r_val & 0xF) < 0);
        m_f.set(flag::c, m_a < r_val);
    }

    void processor::cp_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u16 result = m_a - data;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (data & 0xF)) < 0);
        m_f.set(flag::c, m_a < data);
    }

    void processor::cp_n()
    {
        const u8 n = read_operand();
        const u16 result = m_a - n;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, ((m_a & 0xF) - (n & 0xF)) < 0);
        m_f.set(flag::c, m_a < n);
    }

    void processor::inc_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = r_val + 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, (r_val & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void processor::inc_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const u16 result = data + 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, (data & 0xF) == 0xF);
        write8(addr, static_cast<u8>(result));
    }

    void processor::dec_r(reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = r_val - 1;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (result & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void processor::dec_mem_hl()
    {
        const u16 addr = reg(reg16::hl);
        const u8 data = m_mmu->read8(addr);
        const u16 result = data - 1;
        m_f.set(flag::z, (result & 0xFF) == 0);
        m_f.set(flag::n, true);
        m_f.set(flag::h, (result & 0xF) == 0xF);
        write8(addr, static_cast<u8>(result));
    }

    void processor::and_r(reg8 r)
    {
        const u8 result = m_a & reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::and_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a & data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::and_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a & n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, true);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::or_r(reg8 r)
    {
        const u8 result = m_a | reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::or_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a | data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::or_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a | n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::xor_r(reg8 r)
    {
        const u8 result = m_a ^ reg(r);
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::xor_mem_hl()
    {
        const u8 data = m_mmu->read8(reg(reg16::hl));
        const u8 result = m_a ^ data;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::xor_n()
    {
        const u8 n = read_operand();
        const u8 result = m_a ^ n;
        m_f.set(flag::z, result == 0);
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, false);
        m_a = result;
    }

    void processor::ccf()
    {
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, !m_f.c());
    }

    void processor::scf()
    {
        m_f.set(flag::n, false);
        m_f.set(flag::h, false);
        m_f.set(flag::c, true);
    }

    void processor::cpl()
    {
        m_a = ~m_a;
        m_f.set(flag::n, true);
        m_f.set(flag::h, true);
    }

    // 16-bit loads
    void processor::ld_rr_nn(reg16 dst) { set_reg(dst, read_operands()); }

    void processor::ld_mem_nn_sp() { write16(read_operands(), m_sp); }

    void processor::ld_sp_hl() { m_sp = reg(reg16::hl); }

    void processor::push_rr(reg16 src) { push16(reg(src)); }

    void processor::pop_rr(reg16 dst) { set_reg(dst, pop16()); }

    void processor::ld_hl_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, ((m_sp & 0xF) + (e & 0xF)) > 0xF);
        m_f.set(flag::c, ((m_sp & 0xFF) + (e & 0xFF)) > 0xFF);
        set_reg(reg16::hl, m_sp + static_cast<u16>(e));
    }

    void processor::nop() {}

} // namespace cb
