#pragma once

#include <utility>

#include "core/cartridge.hpp"
#include "core/timer.hpp"
#include "core/video/ppu.hpp"

namespace gb
{
struct Bus
{
    Cartridge cartridge{};
    Ppu ppu;
    Timer timer_;
    std::vector<uint8_t> wram;
    std::array<uint8_t, 128> hram;
    uint8_t interrupt_enable{0x00};
    uint8_t interrupt_flag{0xe1};

#ifdef GBCXX_TESTS
    // NOLINTNEXTLINE
    explicit Bus(std::vector<uint8_t> /*rom_data*/) : wram(64_KiB) {}
#else
    explicit Bus(std::vector<uint8_t> rom_data)
        : cartridge(Cartridge::MakeFromRom(std::move(rom_data))), wram(8_KiB)
    {
    }
#endif

    void Tick(uint8_t tcycles);

    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);
};

}  // namespace gb
