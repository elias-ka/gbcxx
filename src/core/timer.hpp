#pragma once

#include <cstdint>

namespace gb
{
class Core;

class Timer
{
public:
    explicit Timer(Core& gb) : gb_(gb) {}

    [[nodiscard]] uint8_t Read(uint16_t addr) const;
    void Write(uint16_t addr, uint8_t val);

    void Tick();

private:
    void SysclkWrite(uint8_t new_val);
    void DetectEdge(uint8_t before, uint8_t after);

    Core& gb_;
    uint8_t tima_{0x00};
    bool tima_reload_cycle_{};
    uint8_t tma_{0x00};
    uint8_t tac_{0xf8};
    uint8_t last_bit_{};
    uint8_t sysclk_{};
    uint8_t cycles_til_tima_irq_{};
};
}  // namespace gb
