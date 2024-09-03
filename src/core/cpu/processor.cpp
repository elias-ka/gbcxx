#include "core/cpu/processor.h"
#include "common/error.h"
#include "common/try.h"
#include "core/memory/mmu.h"
#include <cstdint>

namespace core::cpu
{
    tl::expected<uint8_t, common::error> processor::fetch_imm8(const memory::mmu& mmu)
    {
        return mmu.read8(++m_reg.pc);
    }

    tl::expected<uint16_t, common::error> processor::fetch_imm16(const memory::mmu& mmu)
    {
        const auto lo = TRY(fetch_imm8(mmu));
        const auto hi = TRY(fetch_imm8(mmu));
        return (hi << 8) | lo;
    }

} // namespace core::cpu
