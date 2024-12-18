#pragma once

#include <cstdint>
#include <memory>

#include "core/mbc.hpp"
#include "core/timer.hpp"
#include "core/video/ppu.hpp"

namespace gbcxx {
class Mmu {
public:
    explicit Mmu(std::unique_ptr<Mbc> mbc);

    auto tick() -> void;

    [[nodiscard]] auto read(uint16_t address) const -> uint8_t;
    [[nodiscard]] auto read16(uint16_t address) const -> uint16_t;

    auto write(uint16_t address, uint8_t value) -> void;
    auto write16(uint16_t address, uint16_t value) -> void;

    auto resize_memory(size_t new_size) -> void
    {
        m_wram.resize(new_size);
    }

    auto ppu() -> Ppu&
    {
        return m_ppu;
    }

private:
    Ppu m_ppu;
    Timer m_timer;
    std::array<uint8_t, 0x100> m_bootrom;
    std::vector<uint8_t> m_wram;
    std::vector<uint8_t> m_hram;
    std::unique_ptr<Mbc> m_mbc;
    bool m_bootrom_enabled { false };
    uint8_t m_inte { 0x00 };
    uint8_t m_intf { 0xE1 };
};

} // namespace gbcxx
