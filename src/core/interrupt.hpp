#pragma once

#include <cstdint>

namespace gbcxx {
enum class Interrupt : uint8_t {
    vblank = 1 << 0,
    stat = 1 << 1,
    timer = 1 << 2,
    serial = 1 << 3,
    joypad = 1 << 4,
};
} // namespace gbcxx
