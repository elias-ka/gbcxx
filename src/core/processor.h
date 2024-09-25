#pragma once

#include "core/memory.h"
#include "core/video.h"
#include "util.h"
#include <array>
#include <fmt/base.h>
#include <string>

namespace cb::cpu
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
    };

    struct registers
    {
        u16 pc;
        u16 sp;
        u8 a;
        flags f;
        u8 b;
        u8 c;
        u8 d;
        u8 e;
        u8 h;
        u8 l;

        u16 read16(reg16 reg) const;
        void write16(reg16 reg, u16 value);
    };

    class processor
    {
    public:
        void step(memory::mmu& mmu);
        usz cycles_elapsed() const { return m_cycles_elapsed; }
        registers& reg() { return m_reg; }

    private:
        void tick(memory::mmu& mmu);
        void tick4(memory::mmu& mmu);

        u8 fetch_imm8(memory::mmu& mmu);
        u16 fetch_imm16(memory::mmu& mmu);

        void execute(u8 opcode);

        void nop();

    private:
        registers m_reg{};
        usz m_cycles_elapsed{};
    };
} // namespace cb::cpu
