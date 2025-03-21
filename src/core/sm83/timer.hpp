#pragma once

#include <cstdint>
#include <utility>

namespace gb::sm83
{
class Timer
{
public:
    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    void Tick(uint8_t tcycles);
    uint8_t ConsumeInterrupts() { return std::exchange(interrupts_, 0); }

private:
    uint8_t div_{0xab};
    uint8_t tima_{0};
    uint8_t tma_{0};
    uint8_t tac_{0xf8};
    uint16_t internal_div_{};
    uint16_t internal_tima_{};
    uint8_t interrupts_;
};
}  // namespace gb::sm83
