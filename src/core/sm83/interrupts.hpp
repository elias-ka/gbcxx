#pragma once

#include <bitset>
#include <cstdint>
#include <utility>

namespace gb::sm83
{
class Interrupts
{
public:
    Interrupts() = default;
    explicit Interrupts(uint8_t byte) : bits_(byte) {}

    void SetVBlank(bool set = true) { bits_[0] = set; }
    void SetStat(bool set = true) { bits_[1] = set; }
    void SetTimer(bool set = true) { bits_[2] = set; }
    void SetSerial(bool set = true) { bits_[3] = set; }
    void SetJoypad(bool set = true) { bits_[4] = set; }

    uint8_t Consume()
    {
        const auto interrupts = std::exchange(bits_, {});
        return static_cast<uint8_t>(interrupts.to_ulong());
    }

private:
    std::bitset<8> bits_;
};
}  // namespace gb::sm83
