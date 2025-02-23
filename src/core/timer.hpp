#pragma once

#include <cstdint>
#include <utility>

namespace gb
{
class Timer
{
public:
    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    void Tick(uint8_t tcycles);
    uint8_t GetAndClearInterrupts() { return std::exchange(interrupts_, 0); }

private:
    uint16_t div_counter_{};
    uint16_t tima_counter_{};
    uint16_t tac_cycles_{1024};
    uint8_t interrupts_{};
    bool enabled_{};
    bool tima_overflow_{};

    uint8_t div_{0xab};
    uint8_t tima_{0};
    uint8_t tma_{0};
    uint8_t tac_{0xf8};
};
}  // namespace gb
