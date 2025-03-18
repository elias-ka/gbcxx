#pragma once

#include "core/joypad.hpp"
#include "core/memory/cartridge.hpp"
#include "core/sm83/timer.hpp"
#include "core/video/ppu.hpp"

namespace gb::memory
{
struct Bus
{
    Cartridge cartridge{};
    video::Ppu ppu;
    sm83::Timer timer;
    Joypad joypad;
    std::vector<uint8_t> wram;
    std::array<uint8_t, 128> hram;
    uint8_t interrupt_enable{0x00};
    uint8_t interrupt_flag{0xe1};

#ifdef GBCXX_TESTS
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    explicit Bus(std::vector<uint8_t> /*rom_data*/) : wram(64_KiB) {}
#else
    explicit Bus(std::vector<uint8_t> rom_data)
        : cartridge(Cartridge::FromRom(std::move(rom_data))), wram(8_KiB)
    {
    }
#endif

    void Tick(uint8_t tcycles);

    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    [[nodiscard]] uint8_t GetPendingInterrupts() const
    {
        return interrupt_enable & interrupt_flag & 0x1f;
    }
};

}  // namespace gb::memory
