#include "core/cpu/processor.h"
#include "common/error.h"
#include "common/try.h"
#include "core/memory/mmu.h"
#include <cstdint>

namespace core::cpu
{
    tl::expected<uint8_t, common::error> processor::fetch_imm8(const memory::mmu& mmu)
    {
        return mmu.read8(m_reg.pc++);
    }

    tl::expected<uint16_t, common::error> processor::fetch_imm16(const memory::mmu& mmu)
    {
        const auto lo = TRY(fetch_imm8(mmu));
        const auto hi = TRY(fetch_imm8(mmu));
        return (hi << 8) | lo;
    }

    tl::expected<int, common::error> processor::step(const memory::mmu& mmu)
    {
        const auto opcode = TRY(fetch_imm8(mmu));
        return 0;
    }

    void processor::nop() {}

    void processor::execute(uint8_t opcode)
    {
        // https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
        const uint8_t x = opcode >> 6;           // bits 7-6
        const uint8_t y = (opcode >> 3) & 0b111; // bits 5-3
        const uint8_t z = opcode & 0b111;        // bits 2-0
        const uint8_t p = y >> 1;                // bits 5-4
        const uint8_t q = y & 1;                 // bit 3

        if (x == 0)
        {
            if (z == 0)
            {
                switch (y)
                {
                case 0: nop();
                }
            }
        }
    }

} // namespace core::cpu
