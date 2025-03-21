#pragma once

#include <cstdint>

namespace gb::sm83
{
enum Interrupt : uint8_t
{
    IntVBlank = 1 << 0,
    IntLcd = 1 << 1,
    IntTimer = 1 << 2,
    IntSerial = 1 << 3,
    IntJoypad = 1 << 4
};

}  // namespace gb::sm83
