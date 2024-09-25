#include "core/processor.h"
#include "core/memory.h"
#include "util.h"
#include <fmt/base.h>

namespace cb::cpu
{
    namespace
    {
        constexpr u8 z_mask = 1 << 7;
        constexpr u8 n_mask = 1 << 6;
        constexpr u8 h_mask = 1 << 5;
        constexpr u8 c_mask = 1 << 4;
    } // namespace

    bool flags::z() const { return raw & z_mask; }
    bool flags::n() const { return raw & n_mask; }
    bool flags::h() const { return raw & h_mask; }
    bool flags::c() const { return raw & c_mask; }

    void flags::set(flag f, bool set)
    {
        switch (f)
        {
        case flag::z: set ? (raw |= z_mask) : (raw &= ~z_mask); break;
        case flag::n: set ? (raw |= n_mask) : (raw &= ~n_mask); break;
        case flag::h: set ? (raw |= h_mask) : (raw &= ~h_mask); break;
        case flag::c: set ? (raw |= c_mask) : (raw &= ~c_mask); break;
        }
    }

    u16 registers::read16(reg16 reg) const
    {
        switch (reg)
        {
        case reg16::af: return static_cast<u16>(a << 8) | f.raw;
        case reg16::bc: return static_cast<u16>(b << 8) | c;
        case reg16::de: return static_cast<u16>(d << 8) | e;
        case reg16::hl: return static_cast<u16>(h << 8) | l;
        }
    }

    void registers::write16(reg16 reg, u16 value)
    {
        switch (reg)
        {
        case reg16::af:
            a = static_cast<u8>(value >> 8);
            f.raw = static_cast<u8>(value & 0x00F0);
            break;
        case reg16::bc:
            b = static_cast<u8>(value >> 8);
            c = static_cast<u8>(value & 0x00FF);
            break;
        case reg16::de:
            d = static_cast<u8>(value >> 8);
            e = static_cast<u8>(value & 0x00FF);
            break;
        case reg16::hl:
            h = static_cast<u8>(value >> 8);
            l = static_cast<u8>(value & 0x00FF);
            break;
        }
    }

    void processor::step(memory::mmu& mmu)
    {
        m_cycles_elapsed = 0;
        const auto opcode = fetch_imm8(mmu);
        execute(opcode);
    }

    u8 processor::fetch_imm8(memory::mmu& mmu)
    {
        tick4(mmu);
        return mmu.read8(m_reg.pc++);
    }

    u16 processor::fetch_imm16(memory::mmu& mmu)
    {
        const auto lo = fetch_imm8(mmu);
        const auto hi = fetch_imm8(mmu);
        return static_cast<u16>((hi << 8) | lo);
    }

    void processor::tick(memory::mmu& mmu)
    {
        m_cycles_elapsed += 1;
        mmu.tick_components();
    }
    void processor::tick4(memory::mmu& mmu)
    {
        tick(mmu);
        tick(mmu);
        tick(mmu);
        tick(mmu);
    }

    void processor::nop() {}

    void processor::execute(u8 opcode)
    {
        // https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
        const u8 x = opcode >> 6;           // bits 7-6
        const u8 y = (opcode >> 3) & 0b111; // bits 5-3
        const u8 z = opcode & 0b111;        // bits 2-0
        const u8 p = y >> 1;                // bits 5-4
        const u8 q = y & 1;                 // bit 3
        if (x == 0)
        {
            if (z == 0)
            {
                switch (y)
                {
                case 0: return nop();
                }
            }
        }

        LOG_UNIMPLEMENTED("opcode {:#02x}", opcode);
    }

} // namespace cb::cpu
