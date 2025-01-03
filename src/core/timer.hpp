#pragma once

#include <cstdint>

namespace gbcxx {
class Timer {
 public:
  [[nodiscard]] uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  void tick();

  uint8_t get_and_clear_interrupts() {
    const uint8_t ints = m_interrupts;
    m_interrupts = 0;
    return ints;
  }

 private:
  void sysclk_write(uint8_t new_value);
  void detect_edge(uint8_t before, uint8_t after);

  uint8_t m_tima{};
  bool m_tima_reload_cycle{};
  uint8_t m_tma{};
  uint8_t m_tac{};
  uint8_t m_last_bit{};
  uint8_t m_sysclk{};
  uint8_t m_cycles_til_tima_irq{};
  uint8_t m_interrupts{};
};
}  // namespace gbcxx
