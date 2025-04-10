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

class LcdStatus : public std::bitset<8>
{
public:
    using std::bitset<8>::bitset;

    explicit operator uint8_t() const { return static_cast<uint8_t>(to_ulong()); }

    [[nodiscard]] constexpr Mode GetMode() const
    {
        return static_cast<Mode>(static_cast<uint8_t>(*this) & 0b11);
    }
    [[nodiscard]] constexpr bool CompareFlag() const { return test(2); }
    [[nodiscard]] constexpr bool Mode0Condition() const { return test(3); }
    [[nodiscard]] constexpr bool Mode1Condition() const { return test(4); }
    [[nodiscard]] constexpr bool Mode2Condition() const { return test(5); }
    [[nodiscard]] constexpr bool LycEqLyEnable() const { return test(6); }

    constexpr void SetMode(Mode mode)
    {
        *this &= 0b1111'1100;
        *this |= std::to_underlying(mode);
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
    constexpr void SetCompareFlag(bool on = true) { set(2, on); }
    constexpr void SetMode0Condition(bool on = true) { set(3, on); }
    constexpr void SetMode1Condition(bool on = true) { set(4, on); }
    constexpr void SetMode2Condition(bool on = true) { set(5, on); }
    constexpr void SetLycEqLyEnable(bool on = true) { set(6, on); }
};
}  // namespace gb::video
