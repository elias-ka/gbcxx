#pragma once

#include "core/mbc.hpp"
#include "core/memory.hpp"
#include "util.hpp"
#include <array>
#include <fmt/base.h>
#include <string>

namespace cb
{
    enum class Flag
    {
        z,
        n,
        h,
        c
    };

    struct Flags
    {
        u8 raw;

        bool operator==(const Flags&) const = default;

        bool z() const;
        bool n() const;
        bool h() const;
        bool c() const;

        void set(Flag f, bool set = true);
    };

    inline std::string format_as(Flags f)
    {
        const std::array<char, 4> symbols = {f.z() ? 'Z' : '-', f.n() ? 'N' : '-',
                                             f.h() ? 'H' : '-', f.c() ? 'C' : '-'};
        return {symbols.begin(), symbols.end()};
    }

    enum class Reg8
    {
        a,
        b,
        c,
        d,
        e,
        h,
        l
    };

    enum class Reg16
    {
        af,
        bc,
        de,
        hl,
        sp
    };

    enum class Condition
    {
        nz,
        z,
        nc,
        c
    };

    class Registers
    {
    public:
        Registers() = default;
        explicit Registers(u16 sp, Flags f, u8 a, u8 b, u8 c, u8 d, u8 e, u8 h, u8 l)
            : m_sp(sp)
            , m_f(f)
            , m_a(a)
            , m_b(b)
            , m_c(c)
            , m_d(d)
            , m_e(e)
            , m_h(h)
            , m_l(l)
        {
        }

        Flags f() const { return m_f; }
        Flags& f() { return m_f; }

        u8 get(Reg8 reg) const;
        u16 get(Reg16 reg) const;

        void set(Reg8 reg, u8 val);
        void set(Reg16 reg, u16 val);

    private:
        u16 m_sp{0xFFFE};
        Flags m_f{BIT(7)};
        u8 m_a{0x01};
        u8 m_b{0x00};
        u8 m_c{0x13};
        u8 m_d{0x00};
        u8 m_e{0xD8};
        u8 m_h{0x01};
        u8 m_l{0x4D};
    };

    class Cpu
    {
    public:
        explicit Cpu(MbcVariant mbc)
            : m_mmu(std::move(mbc))
        {
        }

        Cpu(MbcVariant mbc, u16 pc, u16 sp, Flags f, u8 a, u8 b, u8 c, u8 d, u8 e, u8 h, u8 l)
            : m_mmu(std::move(mbc))
            , m_regs(sp, f, a, b, c, d, e, h, l)
            , m_pc(pc)
        {
        }

        void step();
        usz cycles_elapsed() const { return m_cycles_elapsed; }

        u16 pc() const { return m_pc; }
        u16 sp() const { return m_regs.get(Reg16::sp); }
        Flags flags() const { return m_regs.f(); }

        u8 reg(Reg8 reg) const { return m_regs.get(reg); }
        u16 reg(Reg16 reg) const { return m_regs.get(reg); }
        void set_reg(Reg8 reg, u8 val) { m_regs.set(reg, val); }
        void set_reg(Reg16 reg, u16 val) { m_regs.set(reg, val); }

        Mmu& mmu() { return m_mmu; }

    private:
        void tick();
        void tick4();

        u8 cycle_read8(u16 addr);
        u16 cycle_read16(u16 addr);
        void cycle_write8(u16 addr, u8 data);
        void cycle_write16(u16 addr, u16 data);

        u8 read_operand();
        u16 read_operands();

        void push8(u8 value);
        void push16(u16 value);
        u8 pop8();
        u16 pop16();

        void handle_interrupts();
        void execute(u8 opcode);

        bool check_condition(Condition cond) const
        {
            switch (cond)
            {
            case Condition::nz: return !m_regs.f().z();
            case Condition::z: return m_regs.f().z();
            case Condition::nc: return !m_regs.f().c();
            case Condition::c: return m_regs.f().c();
            }
        }

    private:
        Mmu m_mmu;
        Registers m_regs{};
        usz m_cycles_elapsed{};
        u16 m_pc{0x100};
        bool m_ime{};
        bool m_ime_pending{};
        bool m_halted{};

    private:
        // 8-bit loads
        void ld_r_r(Reg8 dst, Reg8 src);
        void ld_r_n(Reg8 dst);
        void ld_r_mem_hl(Reg8 dst);
        void ld_mem_hl_r(Reg8 src);
        void ld_mem_hl_n();
        void ld_a_mem_rr(Reg16 src);
        void ld_mem_rr_a(Reg16 dst);
        void ld_a_mem_nn();
        void ld_mem_nn_a();
        void ldh_a_mem_c();
        void ldh_mem_c_a();
        void ldh_a_mem_n();
        void ldh_mem_n_a();
        void ld_a_mem_hl_dec();
        void ld_mem_hl_dec_a();
        void ld_a_mem_hl_inc();
        void ld_mem_hl_inc_a();

        // 8-bit arithmetic and logical
        void add_r(Reg8 r);
        void add_mem_hl();
        void add_n();
        void adc_r(Reg8 r);
        void adc_mem_hl();
        void adc_n();
        void sub_r(Reg8 r);
        void sub_mem_hl();
        void sub_n();
        void sbc_r(Reg8 r);
        void sbc_mem_hl();
        void sbc_n();
        void cp_r(Reg8 r);
        void cp_mem_hl();
        void cp_n();
        void inc_r(Reg8 r);
        void inc_mem_hl();
        void dec_r(Reg8 r);
        void dec_mem_hl();
        void and_r(Reg8 r);
        void and_mem_hl();
        void and_n();
        void or_r(Reg8 r);
        void or_mem_hl();
        void or_n();
        void xor_r(Reg8 r);
        void xor_mem_hl();
        void xor_n();
        void ccf();
        void scf();
        void daa();
        void cpl();

        // 16-bit loads
        void ld_rr_nn(Reg16 dst);
        void ld_mem_nn_sp();
        void ld_sp_hl();
        void push_rr(Reg16 src);
        void pop_rr(Reg16 dst);
        void ld_hl_sp_e();

        // 16-bit arithmetic
        void inc_rr(Reg16 rr);
        void dec_rr(Reg16 rr);
        void add_hl_rr(Reg16 rr);
        void add_sp_e();

        // Control flow
        void jp_nn();
        void jp_hl();
        void jp_cc_nn(Condition cc);
        void jr_e();
        void jr_cc_e(Condition cc);
        void call_nn();
        void call_cc_nn(Condition cc);
        void ret();
        void ret_cc(Condition cc);
        void reti();
        void rst_n(u8 vec);

        // Miscellaneous
        void halt();
        void stop();
        void di();
        void ei();
        void nop();

        // Rotate, shift, and bit operations
        void rlca();
        void rrca();
        void rla();
        void rra();
        void rlc_r(Reg8 r);
        void rlc_mem_hl();
        void rrc_r(Reg8 r);
        void rrc_mem_hl();
        void rl_r(Reg8 r);
        void rl_mem_hl();
        void rr_r(Reg8 r);
        void rr_mem_hl();
        void sla_r(Reg8 r);
        void sla_mem_hl();
        void sra_r(Reg8 r);
        void sra_mem_hl();
        void swap_r(Reg8 r);
        void swap_mem_hl();
        void srl_r(Reg8 r);
        void srl_mem_hl();
        void bit_b_r(u8 b, Reg8 r);
        void bit_b_mem_hl(u8 b);
        void res_b_r(u8 b, Reg8 r);
        void res_b_mem_hl(u8 b);
        void set_b_r(u8 b, Reg8 r);
        void set_b_mem_hl(u8 b);
    };
} // namespace cb
