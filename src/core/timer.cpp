#include "core/timer.hpp"

#include "core/constants.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gbcxx {
uint8_t Timer::read(uint16_t address) const {
  switch (address) {
    case REG_DIV: return m_div >> 8;
    case REG_TIMA: return m_tima;
    case REG_TMA: return m_tma;
    case REG_TAC: return m_tac & 0b111;
  }
  LOG_WARN("Timer: read from unhandled address {:#06x}", address);
  return 0;
}

void Timer::write(uint16_t address, uint8_t value) {
  switch (address) {
    case REG_DIV: m_div = 0; break;
    case REG_TIMA: m_tima = value; break;
    case REG_TMA: m_tma = value; break;
    case REG_TAC: m_tac = value & 0b111; break;
  }
}

void Timer::tick() {
  while (++m_internal_div >= 256) {
    m_internal_div -= 256;
    m_div++;
  }

  const bool timer_enabled = m_tac & 0b100;
  const uint8_t clock_select = m_tac & 0b11;
  const size_t input_clock = [clock_select] {
    switch (clock_select) {
      case 0: return 1024UZ;
      case 1: return 16UZ;
      case 2: return 64UZ;
      case 3: return 256UZ;
      default: {
        LOG_ERROR("Timer: invalid clock select value {}", clock_select);
        return 0UZ;
      }
    }
  }();

  if (timer_enabled) {
    while (++m_internal_tima >= input_clock) {
      m_internal_tima -= input_clock;
      if (++m_tima == 0) {
        m_tima = m_tma;
        m_interrupts |= std::to_underlying(Interrupt::Timer);
      }
    }
  }
}
}  // namespace gbcxx
