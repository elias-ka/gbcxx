#include "processor.hpp"

// to-do: a lot of duplication in this file can be reduced

namespace cb
{
    // 8-bit loads
    void Cpu::ld_r_r(Reg8 dest, Reg8 src) { set_reg(dest, reg(src)); }

    void Cpu::ld_r_n(Reg8 dest) { set_reg(dest, read_operand()); }

    void Cpu::ld_r_mem_hl(Reg8 dest) { set_reg(dest, cycle_read8(reg(Reg16::hl))); }

    void Cpu::ld_mem_hl_r(Reg8 src) { cycle_write8(reg(Reg16::hl), reg(src)); }

    void Cpu::ld_mem_hl_n() { cycle_write8(reg(Reg16::hl), read_operand()); }

    void Cpu::ld_a_mem_rr(Reg16 src) { m_regs.set(Reg8::a, cycle_read8(reg(src))); }

    void Cpu::ld_mem_rr_a(Reg16 dst) { cycle_write8(reg(dst), reg(Reg8::a)); }

    void Cpu::ld_a_mem_nn() { m_regs.set(Reg8::a, cycle_read8(read_operands())); }

    void Cpu::ld_mem_nn_a()
    {
        const auto addr = read_operands();
        cycle_write8(addr, reg(Reg8::a));
    }

    void Cpu::ldh_a_mem_c()
    {
        const auto addr = static_cast<u16>(0xFF00 + reg(Reg8::c));
        set_reg(Reg8::a, cycle_read8(addr));
    }

    void Cpu::ldh_mem_c_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + reg(Reg8::c));
        cycle_write8(addr, reg(Reg8::a));
    }

    void Cpu::ldh_a_mem_n()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        set_reg(Reg8::a, cycle_read8(addr));
    }

    void Cpu::ldh_mem_n_a()
    {
        const auto addr = static_cast<u16>(0xFF00 + read_operand());
        cycle_write8(addr, reg(Reg8::a));
    }

    void Cpu::ld_a_mem_hl_dec()
    {
        const auto hl = reg(Reg16::hl);
        set_reg(Reg8::a, cycle_read8(hl));
        set_reg(Reg16::hl, hl - 1);
    }

    void Cpu::ld_mem_hl_dec_a()
    {
        const auto hl = reg(Reg16::hl);
        cycle_write8(hl, reg(Reg8::a));
        set_reg(Reg16::hl, hl - 1);
    }

    void Cpu::ld_a_mem_hl_inc()
    {
        const auto hl = reg(Reg16::hl);
        set_reg(Reg8::a, cycle_read8(hl));
        set_reg(Reg16::hl, hl + 1);
    }

    void Cpu::ld_mem_hl_inc_a()
    {
        const auto hl_val = reg(Reg16::hl);
        cycle_write8(hl_val, reg(Reg8::a));
        set_reg(Reg16::hl, hl_val + 1);
    }

    // 8-bit arithmetic and logical
    void Cpu::add_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = reg(Reg8::a) + r_val;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (r_val & 0xF)) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::add_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u16 result = reg(Reg8::a) + data;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (data & 0xF)) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::add_n()
    {
        const u8 n = read_operand();
        const u16 result = reg(Reg8::a) + n;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (n & 0xF)) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::adc_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = reg(Reg8::a) + r_val + m_regs.f().c();
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (r_val & 0xF) + m_regs.f().c()) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::adc_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u16 result = reg(Reg8::a) + data + m_regs.f().c();
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (data & 0xF) + m_regs.f().c()) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::adc_n()
    {
        const u8 n = read_operand();
        const u16 result = reg(Reg8::a) + n + m_regs.f().c();
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) + (n & 0xF) + m_regs.f().c()) > 0xF);
        m_regs.f().set(Flag::c, result > 0xFF);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::sub_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = reg(Reg8::a) - r_val;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, (reg(Reg8::a) & 0xF) - (r_val & 0xF) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < r_val);
        set_reg(Reg8::a, result);
    }

    void Cpu::sub_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u16 result = reg(Reg8::a) - data;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (data & 0xF)) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < data);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::sub_n()
    {
        const u8 n = read_operand();
        const u16 result = reg(Reg8::a) - n;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (n & 0xF)) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < n);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::sbc_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const int result = reg(Reg8::a) - r_val - m_regs.f().c();
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (r_val & 0xF) - m_regs.f().c()) < 0);
        m_regs.f().set(Flag::c, result < 0);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::sbc_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const int result = reg(Reg8::a) - data - m_regs.f().c();
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (data & 0xF) - m_regs.f().c()) < 0);
        m_regs.f().set(Flag::c, result < 0);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::sbc_n()
    {
        const u8 n = read_operand();
        const int result = reg(Reg8::a) - n - m_regs.f().c();
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (n & 0xF) - m_regs.f().c()) < 0);
        m_regs.f().set(Flag::c, result < 0);
        set_reg(Reg8::a, static_cast<u8>(result));
    }

    void Cpu::cp_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = reg(Reg8::a) - r_val;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, (reg(Reg8::a) & 0xF) - (r_val & 0xF) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < r_val);
    }

    void Cpu::cp_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u16 result = reg(Reg8::a) - data;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (data & 0xF)) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < data);
    }

    void Cpu::cp_n()
    {
        const u8 n = read_operand();
        const u16 result = reg(Reg8::a) - n;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, ((reg(Reg8::a) & 0xF) - (n & 0xF)) < 0);
        m_regs.f().set(Flag::c, reg(Reg8::a) < n);
    }

    void Cpu::inc_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u16 result = r_val + 1;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, (r_val & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void Cpu::inc_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const u16 result = data + 1;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, (data & 0xF) == 0xF);
        cycle_write8(addr, static_cast<u8>(result));
    }

    void Cpu::dec_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = r_val - 1;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, (result & 0xF) == 0xF);
        set_reg(r, static_cast<u8>(result));
    }

    void Cpu::dec_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const u16 result = data - 1;
        m_regs.f().set(Flag::z, (result & 0xFF) == 0);
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, (result & 0xF) == 0xF);
        cycle_write8(addr, static_cast<u8>(result));
    }

    void Cpu::and_r(Reg8 r)
    {
        const u8 result = reg(Reg8::a) & reg(r);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, true);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::and_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u8 result = reg(Reg8::a) & data;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, true);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::and_n()
    {
        const u8 n = read_operand();
        const u8 result = reg(Reg8::a) & n;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, true);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::or_r(Reg8 r)
    {
        const u8 result = reg(Reg8::a) | reg(r);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::or_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u8 result = reg(Reg8::a) | data;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::or_n()
    {
        const u8 n = read_operand();
        const u8 result = reg(Reg8::a) | n;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::xor_r(Reg8 r)
    {
        const u8 result = reg(Reg8::a) ^ reg(r);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::xor_mem_hl()
    {
        const u8 data = cycle_read8(reg(Reg16::hl));
        const u8 result = reg(Reg8::a) ^ data;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::xor_n()
    {
        const u8 n = read_operand();
        const u8 result = reg(Reg8::a) ^ n;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(Reg8::a, result);
    }

    void Cpu::ccf()
    {
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, !m_regs.f().c());
    }

    void Cpu::scf()
    {
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, true);
    }

    void Cpu::daa()
    {
        // from
        // https://forums.nesdev.org/viewtopic.php?p=196282&sid=38a75719934a07d0ae8ac78a3e1448ad#p196282
        if (!m_regs.f().n())
        {
            if (m_regs.f().c() || reg(Reg8::a) > 0x99)
            {
                set_reg(Reg8::a, reg(Reg8::a) + 0x60);
                m_regs.f().set(Flag::c, true);
            }
            if (m_regs.f().h() || (reg(Reg8::a) & 0x0f) > 0x09)
            {
                set_reg(Reg8::a, reg(Reg8::a) + 0x6);
            }
        }
        else
        {
            if (m_regs.f().c())
                set_reg(Reg8::a, reg(Reg8::a) - 0x60);

            if (m_regs.f().h())
                set_reg(Reg8::a, reg(Reg8::a) - 0x6);
        }

        m_regs.f().set(Flag::z, reg(Reg8::a) == 0);
        m_regs.f().set(Flag::h, false);
    }

    void Cpu::cpl()
    {
        set_reg(Reg8::a, ~reg(Reg8::a));
        m_regs.f().set(Flag::n, true);
        m_regs.f().set(Flag::h, true);
    }

    // 16-bit loads
    void Cpu::ld_rr_nn(Reg16 dst) { set_reg(dst, read_operands()); }

    void Cpu::ld_mem_nn_sp() { cycle_write16(read_operands(), reg(Reg16::sp)); }

    void Cpu::ld_sp_hl() { set_reg(Reg16::sp, reg(Reg16::hl)); }

    void Cpu::push_rr(Reg16 src) { push16(reg(src)); }

    void Cpu::pop_rr(Reg16 dst) { set_reg(dst, pop16()); }

    void Cpu::ld_hl_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg16::sp) & 0xF) + (e & 0xF)) > 0xF);
        m_regs.f().set(Flag::c, ((reg(Reg16::sp) & 0xFF) + (e & 0xFF)) > 0xFF);
        set_reg(Reg16::hl, reg(Reg16::sp) + static_cast<u16>(e));
    }

    // 16-bit arithmetic
    void Cpu::inc_rr(Reg16 rr) { set_reg(rr, reg(rr) + 1); }

    void Cpu::dec_rr(Reg16 rr) { set_reg(rr, reg(rr) - 1); }

    void Cpu::add_hl_rr(Reg16 rr)
    {
        const u16 rr_val = reg(rr);
        const u16 hl_val = reg(Reg16::hl);
        const int result = hl_val + rr_val;
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, (rr_val & 0xFFF) + (hl_val & 0xFFF) > 0xFFF);
        m_regs.f().set(Flag::c, result > 0xFFFF);
        set_reg(Reg16::hl, static_cast<u16>(result));
    }

    void Cpu::add_sp_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, ((reg(Reg16::sp) & 0xF) + (e & 0xF)) > 0xF);
        m_regs.f().set(Flag::c, ((reg(Reg16::sp) & 0xFF) + (e & 0xFF)) > 0xFF);
        set_reg(Reg16::sp, reg(Reg16::sp) + static_cast<u16>(e));
    }

    // Control flow
    void Cpu::jp_nn()
    {
        const u16 nn = read_operands();
        m_pc = nn;
    }

    void Cpu::jp_hl() { m_pc = reg(Reg16::hl); }

    void Cpu::jp_cc_nn(Condition cc)
    {
        const u16 nn = read_operands();
        if (check_condition(cc))
        {
            m_pc = nn;
            tick4();
        }
    }

    void Cpu::jr_e()
    {
        const auto e = static_cast<s8>(read_operand());
        m_pc += e;
    }

    void Cpu::jr_cc_e(Condition cc)
    {
        const auto e = static_cast<s8>(read_operand());
        if (check_condition(cc))
        {
            m_pc += e;
            tick4();
        }
    }

    void Cpu::call_nn()
    {
        const u16 nn = read_operands();
        push16(m_pc);
        m_pc = nn;
    }

    void Cpu::call_cc_nn(Condition cc)
    {
        const u16 nn = read_operands();
        if (check_condition(cc))
        {
            push16(m_pc);
            m_pc = nn;
            tick4();
        }
    }

    void Cpu::ret() { m_pc = pop16(); }

    void Cpu::ret_cc(Condition cc)
    {
        if (check_condition(cc))
        {
            m_pc = pop16();
            tick4();
        }
    }

    void Cpu::reti()
    {
        m_pc = pop16();
        m_ime = true;
    }

    void Cpu::rst_n(u8 vec)
    {
        push16(m_pc);
        m_pc = vec;
    }

    // Miscellaneous
    void Cpu::halt() { m_halted = true; }
    void Cpu::stop() { /* to-do */ }
    void Cpu::di() { m_ime = false; }
    void Cpu::ei() { m_ime_pending = true; }
    void Cpu::nop() {}

    // Rotate, shift, and bit operations
    void Cpu::rlca()
    {
        const bool carry = reg(Reg8::a) & BIT(7);
        const auto result = static_cast<u8>((reg(Reg8::a) << 1) | carry);
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(Reg8::a, result);
    }

    void Cpu::rrca()
    {
        const bool carry = reg(Reg8::a) & BIT(0);
        const auto result = static_cast<u8>((reg(Reg8::a) >> 1) | (carry << 7));
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(Reg8::a, result);
    }

    void Cpu::rla()
    {
        const auto result = static_cast<u8>((reg(Reg8::a) << 1) | m_regs.f().c());
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, reg(Reg8::a) & BIT(7));
        set_reg(Reg8::a, result);
    }

    void Cpu::rra()
    {
        const auto result = static_cast<u8>((reg(Reg8::a) >> 1) | (m_regs.f().c() << 7));
        m_regs.f().set(Flag::z, false);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, reg(Reg8::a) & BIT(0));
        set_reg(Reg8::a, result);
    }

    void Cpu::rlc_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(7);
        const auto result = static_cast<u8>((r_val << 1) | carry);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(r, result);
    }

    void Cpu::rlc_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const bool carry = data & BIT(7);
        const auto result = static_cast<u8>((data << 1) | carry);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        cycle_write8(addr, result);
    }

    void Cpu::rrc_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(0);
        const auto result = static_cast<u8>((r_val >> 1) | (carry << 7));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(r, result);
    }

    void Cpu::rrc_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const bool carry = data & BIT(0);
        const auto result = static_cast<u8>((data >> 1) | (carry << 7));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        cycle_write8(addr, result);
    }

    void Cpu::rl_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const auto result = static_cast<u8>((r_val << 1) | m_regs.f().c());
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, r_val & BIT(7));
        set_reg(r, result);
    }

    void Cpu::rl_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const auto result = static_cast<u8>((data << 1) | m_regs.f().c());
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, data & BIT(7));
        cycle_write8(addr, result);
    }

    void Cpu::rr_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const auto result = static_cast<u8>((r_val >> 1) | (m_regs.f().c() << 7));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, r_val & BIT(0));
        set_reg(r, result);
    }

    void Cpu::rr_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const auto result = static_cast<u8>((data >> 1) | (m_regs.f().c() << 7));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, data & BIT(0));
        cycle_write8(addr, result);
    }

    void Cpu::sla_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(7);
        const auto result = static_cast<u8>(r_val << 1);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(r, result);
    }

    void Cpu::sla_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const bool carry = data & BIT(7);
        const auto result = static_cast<u8>(data << 1);
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        cycle_write8(addr, result);
    }

    void Cpu::sra_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(0);
        const u8 result = static_cast<u8>((r_val >> 1) | (r_val & BIT(7)));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(r, result);
    }

    void Cpu::sra_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const bool carry = data & BIT(0);
        const u8 result = static_cast<u8>((data >> 1) | (data & BIT(7)));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        cycle_write8(addr, result);
    }

    void Cpu::swap_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const u8 result = static_cast<u8>(((r_val & 0x0F) << 4) | (((r_val & 0xF0) >> 4) & 0xF));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        set_reg(r, result);
    }

    void Cpu::swap_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const u8 result = static_cast<u8>(((data & 0x0F) << 4) | (((data & 0xF0) >> 4) & 0xF));
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, false);
        cycle_write8(addr, result);
    }

    void Cpu::srl_r(Reg8 r)
    {
        const u8 r_val = reg(r);
        const bool carry = r_val & BIT(0);
        const u8 result = r_val >> 1;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        set_reg(r, result);
    }

    void Cpu::srl_mem_hl()
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        const bool carry = data & BIT(0);
        const u8 result = data >> 1;
        m_regs.f().set(Flag::z, result == 0);
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, false);
        m_regs.f().set(Flag::c, carry);
        cycle_write8(addr, result);
    }

    void Cpu::bit_b_r(u8 b, Reg8 r)
    {
        m_regs.f().set(Flag::z, !(reg(r) & BIT(b)));
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, true);
    }

    void Cpu::bit_b_mem_hl(u8 b)
    {
        m_regs.f().set(Flag::z, !(cycle_read8(reg(Reg16::hl)) & BIT(b)));
        m_regs.f().set(Flag::n, false);
        m_regs.f().set(Flag::h, true);
    }

    void Cpu::res_b_r(u8 b, Reg8 r) { set_reg(r, (reg(r) & ~BIT(b))); }

    void Cpu::res_b_mem_hl(u8 b)
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        cycle_write8(addr, data & ~BIT(b));
    }

    void Cpu::set_b_r(u8 b, Reg8 r) { set_reg(r, u8(reg(r) | BIT(b))); }

    void Cpu::set_b_mem_hl(u8 b)
    {
        const u16 addr = reg(Reg16::hl);
        const u8 data = cycle_read8(addr);
        cycle_write8(addr, u8(data | BIT(b)));
    }

} // namespace cb
