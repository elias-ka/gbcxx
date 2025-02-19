#pragma once

#include <cstdint>

namespace gb
{
enum class Interrupt : uint8_t
{
    VBlank = 1 << 0,
    Lcd = 1 << 1,
    Timer = 1 << 2,
    Serial = 1 << 3,
    Joypad = 1 << 4
};
}  // namespace gb
