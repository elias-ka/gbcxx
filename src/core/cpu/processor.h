#pragma once

#include "common/error.h"
#include "core/memory/mmu.h"
#include <cstdint>
#include <tl/expected.hpp>

namespace core::cpu
{
    class processor
    {
    public:
        tl::expected<uint8_t, common::error> fetch_imm8(const memory::mmu& mmu);
        tl::expected<uint16_t, common::error> fetch_imm16(const memory::mmu& mmu);

    private:
        struct
        {
            uint8_t a, f, b, c, d, e, h, l;
            uint16_t sp, pc;
        } m_reg{};
    };
} // namespace core::cpu
