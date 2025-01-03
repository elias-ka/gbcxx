#pragma once

#include <cstdint>

namespace gbcxx {
enum Interrupt : uint8_t {
  vblank = 0,
  stat = 1,
  timer = 2,
  serial = 3,
  joypad = 4
};
}  // namespace gbcxx
