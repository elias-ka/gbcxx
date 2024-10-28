#pragma once

#include "util.hpp"

namespace gbcxx {
enum class Interrupt {
  vblank = 0x00,
  lcd_stat = 0x01,
  timer = 0x02,
  serial = 0x03,
  joypad = 0x04
};

inline u16 get_interrupt_handler_address(Interrupt interrupt) {
  switch (interrupt) {
    case Interrupt::vblank: return 0x0040;
    case Interrupt::lcd_stat: return 0x0048;
    case Interrupt::timer: return 0x0050;
    case Interrupt::serial: return 0x0058;
    case Interrupt::joypad: return 0x0060;
  }
  return 0;
}
}  // namespace gbcxx
