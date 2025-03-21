#pragma once

#include <bitset>
#include <cstdint>
#include <utility>

#include "core/sm83/interrupts.hpp"

namespace gb::video
{
enum class Mode : uint8_t
{
    HBlank = 0,
    VBlank = 1,
    Oam = 2,
    Transfer = 3,
};

class LcdStatus
{
public:
    LcdStatus() = default;
    explicit LcdStatus(uint8_t byte) : bits_(byte) {}

    explicit operator uint8_t() const { return static_cast<uint8_t>(bits_.to_ulong()); }

    [[nodiscard]] constexpr Mode GetMode() const
    {
        return static_cast<Mode>(bits_.to_ulong() & 0b11);
    }
    [[nodiscard]] constexpr bool CompareFlag() const { return bits_.test(2); }
    [[nodiscard]] constexpr bool Mode0Condition() const { return bits_.test(3); }
    [[nodiscard]] constexpr bool Mode1Condition() const { return bits_.test(4); }
    [[nodiscard]] constexpr bool Mode2Condition() const { return bits_.test(5); }
    [[nodiscard]] constexpr bool LycEqLyEnable() const { return bits_.test(6); }

    constexpr void SetMode(Mode mode)
    {
        bits_ &= 0b1111'1100;
        bits_ |= std::to_underlying(mode);
    }
    constexpr void SetMode(Mode mode, uint8_t& interrupts)
    {
        SetMode(mode);
        if ((mode == Mode::HBlank && Mode0Condition()) ||
            (mode == Mode::VBlank && Mode1Condition()) || (mode == Mode::Oam && Mode2Condition()))
        {
            interrupts |= sm83::IntLcd;
        }
    }
    constexpr void SetCompareFlag(bool set = true) { bits_.set(2, set); }
    constexpr void SetMode0Condition(bool set = true) { bits_.set(3, set); }
    constexpr void SetMode1Condition(bool set = true) { bits_.set(4, set); }
    constexpr void SetMode2Condition(bool set = true) { bits_.set(5, set); }
    constexpr void SetLycEqLyEnable(bool set = true) { bits_.set(6, set); }

private:
    std::bitset<8> bits_;
};
}  // namespace gb::video
