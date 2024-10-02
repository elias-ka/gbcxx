#include "processor.h"

namespace cb
{
    // 8-bit loads

    void processor::ld_r_r(reg8 dest, reg8 src) { set_reg(dest, reg(src)); }

    void processor::ld_r_n(reg8 dest) { set_reg(dest, read_operand()); }

    void processor::ld_r_mem_hl(reg8 dest)
    {
        const auto hl = reg(reg16::hl);
        set_reg(dest, m_mmu->read8(hl));
    }

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

    // 16-bit loads

    void processor::ld_rr_nn(reg16 dst) { set_reg(dst, read_operands()); }

    void processor::ld_mem_nn_sp() { write16(read_operands(), m_sp); }

    void processor::ld_sp_hl() { m_sp = reg(reg16::hl); }

    void processor::push_rr(reg16 src) { push16(reg(src)); }

    void processor::pop_rr(reg16 dst) { set_reg(dst, pop16()); }

    void processor::ld_hl_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        set_reg(reg16::hl, m_sp + static_cast<u16>(e));
        m_f.set(flag::z, false);
        m_f.set(flag::n, false);
        m_f.set(flag::h, (((m_sp & 0x0F) + (e & 0x0F)) > 0x0F));
        m_f.set(flag::c, (((m_sp & 0xFF) + (e & 0xFF)) > 0xFF));
    }

    void processor::nop() {}

} // namespace cb
