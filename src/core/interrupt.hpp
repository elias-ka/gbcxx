#pragma once

#include <cstdint>

namespace gb
{
enum class Interrupt : uint8_t
{
    VBlank = 0,
    Stat = 1,
    Timer = 2,
    Serial = 3,
    Joypad = 4
};
}  // namespace gb
