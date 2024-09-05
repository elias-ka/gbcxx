#pragma once

#include "common/error.h"
#include "core/cpu/registers.h"
#include "core/memory/mmu.h"
#include <cstdint>
#include <tl/expected.hpp>

namespace core::cpu
{
    class processor
    {
    public:
        [[nodiscard]] tl::expected<uint8_t, common::error> fetch_imm8(const memory::mmu& mmu);
        [[nodiscard]] tl::expected<uint16_t, common::error> fetch_imm16(const memory::mmu& mmu);

    private:
        registers m_reg;
    };
} // namespace core::cpu
