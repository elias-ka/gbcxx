#pragma once

#include "common/error.h"
#include <cstdint>
#include <tl/expected.hpp>
#include <vector>

namespace core::memory
{
    extern const std::vector<uint8_t> default_bios;

    class mmu
    {
    public:
        mmu(std::vector<uint8_t> bios = default_bios);

        [[nodiscard]] tl::expected<uint8_t, common::error> read8(uint16_t addr) const;
        [[nodiscard]] tl::expected<uint16_t, common::error> read16(uint16_t addr) const;

        void write8(uint16_t addr, uint8_t data);
        void write16(uint16_t addr, uint16_t data);

    private:
        std::vector<uint8_t> m_memory;
        std::vector<uint8_t> m_bios;
        bool m_in_bios{};
    };

} // namespace core::memory
