#pragma once

#include <memory>

#include "core/cpu.hpp"
#include "core/mbc.hpp"
#include "core/ppu.hpp"
#include "core/timer.hpp"

namespace gb
{
class Core
{
public:
    Core() : cpu_(*this), ppu_(*this), timer_(*this), wram_(0x8000) {}

    Ppu& GetPpu() { return ppu_; }

    [[nodiscard]] uint8_t BusRead8(uint16_t addr) const;
    [[nodiscard]] uint16_t BusRead16(uint16_t addr) const;

    void BusWrite8(uint16_t addr, uint8_t val);
    void BusWrite16(uint16_t addr, uint16_t val);

    void RunFrame();
    void Irq(Interrupt interrupt) { cpu_.Irq(interrupt); }
    void LoadRom(const std::filesystem::path& path);
    void ResizeMemory(size_t new_size) { wram_.resize(new_size); }

private:
    Cpu cpu_;
    Ppu ppu_;
    Timer timer_;
    std::vector<uint8_t> wram_;
    std::array<uint8_t, 0x7f> hram_;
    std::unique_ptr<Mbc> mbc_;
    std::string cart_title_;

    uint8_t joyp_{};
    uint8_t sb_{};
    uint8_t sc_{};
};
}  // namespace gb
