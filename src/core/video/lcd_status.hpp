#pragma once

#include <bitset>
#include <cstdint>
#include <utility>

#include "core/interrupt.hpp"

namespace gb
{
enum class Mode : uint8_t
{
    HBlank = 0,
    VBlank = 1,
    Oam = 2,
    Transfer = 3,
};

struct LcdStatus
{
public:
    LcdStatus() = default;
    explicit LcdStatus(uint8_t byte) : bits_(byte) {}
    explicit operator uint8_t() const { return static_cast<uint8_t>(bits_.to_ulong()); }

    [[nodiscard]] constexpr Mode GetMode() const { return Mode(bits_.to_ulong() & 0b11); }
    [[nodiscard]] constexpr bool CompareFlag() const { return bits_.test(2); }
    [[nodiscard]] constexpr bool Mode0Condition() const { return bits_.test(3); }
    [[nodiscard]] constexpr bool Mode1Condition() const { return bits_.test(4); }
    [[nodiscard]] constexpr bool Mode2Condition() const { return bits_.test(5); }
    [[nodiscard]] constexpr bool LycEqLyEnable() const { return bits_.test(6); }

    constexpr void SetMode(Mode m)
    {
        bits_ &= 0b11111100;
        bits_ |= std::to_underlying(m);
    }
    constexpr void SetMode(Mode m, uint8_t& interrupts)
    {
        SetMode(m);
        CheckInterrupts(interrupts);
    }
    constexpr void SetCompareFlag(bool v) { bits_.set(2, v); }
    constexpr void SetMode0Condition(bool v) { bits_.set(3, v); }
    constexpr void SetMode1Condition(bool v) { bits_.set(4, v); }
    constexpr void SetMode2Condition(bool v) { bits_.set(5, v); }
    constexpr void SetLycEqLyEnable(bool v) { bits_.set(6, v); }

private:
    constexpr void CheckInterrupts(uint8_t& interrupts) const
    {
        if ((GetMode() == Mode::HBlank && Mode0Condition()) ||
            (GetMode() == Mode::VBlank && Mode1Condition()) ||
            (GetMode() == Mode::Oam && Mode2Condition()))
        {
            interrupts |= std::to_underlying(Interrupt::Lcd);
        }
    }

    std::bitset<8> bits_;
};
}  // namespace gb
