#pragma once

#include "core/memory.h"
#include "util.h"
#include <array>
#include <fmt/base.h>
#include <string>

namespace cb
{
    enum class flag
    {
        z,
        n,
        h,
        c
    };

    struct flags
    {
        u8 raw;

        bool operator==(const flags&) const = default;

        bool z() const;
        bool n() const;
        bool h() const;
        bool c() const;

        void set(flag f, bool set = true);
    };

    inline std::string format_as(flags f)
    {
        const std::array<char, 4> symbols = {f.z() ? 'Z' : '-', f.n() ? 'N' : '-',
                                             f.h() ? 'H' : '-', f.c() ? 'C' : '-'};
        return {symbols.begin(), symbols.end()};
    }

    enum class reg8
    {
        a,
        b,
        c,
        d,
        e,
        h,
        l
    };

    enum class reg16
    {
        af,
        bc,
        de,
        hl,
        sp
    };

    class processor
    {
    public:
        explicit processor(mmu* mmu)
            : m_mmu(mmu)
        {
        }

        processor(mmu* mmu, u16 pc, u16 sp, flags f, u8 a, u8 b, u8 c, u8 d, u8 e, u8 h, u8 l)
            : m_mmu(mmu)
            , m_pc(pc)
            , m_sp(sp)
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

        void step();
        usz cycles_elapsed() const { return m_cycles_elapsed; }

        u16 pc() const { return m_pc; }
        u16 sp() const { return m_sp; }
        flags f() const { return m_f; }

        u8 reg(reg8 reg) const;
        u16 reg(reg16 reg) const;

        void set_reg(reg8 reg, u8 val);
        void set_reg(reg16 reg, u16 val);

    private:
        void tick();
        void tick4();

        void write8(u16 addr, u8 data);
        void write16(u16 addr, u16 data);

        u8 read_operand();
        u16 read_operands();

        void push8(u8 value);
        void push16(u16 value);
        u8 pop8();
        u16 pop16();

        void execute(u8 opcode);

        // 8-bit loads
        void ld_r_r(reg8 dst, reg8 src);
        void ld_r_n(reg8 dst);
        void ld_r_mem_hl(reg8 dst);
        void ld_mem_hl_r(reg8 src);
        void ld_mem_hl_n();
        void ld_a_mem_rr(reg16 src);
        void ld_mem_rr_a(reg16 dst);
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
        void add_r(reg8 r);
        void add_mem_hl();
        void add_n();
        void adc_r(reg8 r);
        void adc_mem_hl();
        void adc_n();
        void sub_r(reg8 r);
        void sub_mem_hl();
        void sub_n();
        void sbc_r(reg8 r);
        void sbc_mem_hl();
        void sbc_n();
        void cp_r(reg8 r);
        void cp_mem_hl();
        void cp_n();
        void inc_r(reg8 r);
        void inc_mem_hl();
        void dec_r(reg8 r);
        void dec_mem_hl();
        void and_r(reg8 r);
        void and_mem_hl();
        void and_n();
        void or_r(reg8 r);
        void or_mem_hl();
        void or_n();
        void xor_r(reg8 r);
        void xor_mem_hl();
        void xor_n();

        // 16-bit loads
        void ld_rr_nn(reg16 dst);
        void ld_mem_nn_sp();
        void ld_sp_hl();
        void push_rr(reg16 src);
        void pop_rr(reg16 dst);
        void ld_hl_sp_e();

        void nop();

    private:
        usz m_cycles_elapsed{};
        mmu* m_mmu;
        u16 m_pc{};
        u16 m_sp{};
        flags m_f{};
        u8 m_a{};
        u8 m_b{};
        u8 m_c{};
        u8 m_d{};
        u8 m_e{};
        u8 m_h{};
        u8 m_l{};
    };
} // namespace cb
