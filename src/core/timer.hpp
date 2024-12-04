#pragma once

#include <cstddef>
#include <cstdint>

namespace gbcxx {
class Timer {
 public:
  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  void tick();
  uint8_t get_and_clear_interrupts() {
    const uint8_t interrupts = m_interrupts;
    m_interrupts = 0;
    return interrupts;
  }

 private:
  size_t m_internal_div{};
  size_t m_internal_tima{};
  uint8_t m_div{};
  uint8_t m_tima{};
  uint8_t m_tma{};
  uint8_t m_tac{};
  uint8_t m_interrupts{};
};
}  // namespace gbcxx
