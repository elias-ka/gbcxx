#include "core/timer.hpp"

#include "core/constants.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gbcxx {
uint8_t Timer::read(uint16_t address) const {
  LOG_TRACE("Timer::read({:X})", address);
  switch (address) {
    case reg_div: return (m_sysclk >> 6) & 0xff;
    case reg_tima: return m_tima;
    case reg_tma: return m_tma;
    case reg_tac: return m_tac;
    default: {
      DIE("Timer: read from unhandled address {:X}", address);
    }
  }
}

void Timer::write(uint16_t address, uint8_t value) {
  LOG_TRACE("Timer::write({:X}, {:#b})", address, value);
  switch (address) {
    case reg_div: {
      sysclk_write(0);
      break;
    }
    case reg_tima: {
      if (!m_tima_reload_cycle) {
        m_tima = value;
      }
      if (m_cycles_til_tima_irq == 1) {
        m_cycles_til_tima_irq = 0;
      }
      break;
    }
    case reg_tma: {
      if (m_tima_reload_cycle) {
        m_tima = value;
      }
      m_tma = value;
      break;
    }
    case reg_tac: {
      const uint8_t prev_bit = m_last_bit;
      m_last_bit &= ((value & 0x4) >> 2);
      detect_edge(prev_bit, m_last_bit);
      m_tac = value;
      break;
    }
    default: {
      DIE("Timer: Unmapped write {:X} <- {:X}", address, value);
    }
  }
}

void Timer::tick() {
  m_tima_reload_cycle = false;
  if (m_cycles_til_tima_irq > 0) {
    LOG_TRACE("cycles_til_tima_irq: {}", m_cycles_til_tima_irq);
    if (--m_cycles_til_tima_irq == 0) {
      LOG_TRACE("timer IRQ");
      m_interrupts |= Interrupt::timer;
      m_tima = m_tma;
      m_tima_reload_cycle = true;
    }
  }
  sysclk_write(m_sysclk + 1);
}

void Timer::sysclk_write(uint8_t new_value) {
  m_sysclk = new_value;
  auto bit = [this] -> uint8_t {
    switch (m_tac & 0x3) {
      case 3: return (m_sysclk >> 5) & 1;
      case 2: return (m_sysclk >> 3) & 1;
      case 1: return (m_sysclk >> 1) & 1;
      case 0: return (m_sysclk >> 7) & 1;
      default: std::unreachable();
    }
  }();
  bit &= (m_tac & 0x4) >> 2;
  detect_edge(m_last_bit, bit);
  m_last_bit = bit;
}

void Timer::detect_edge(uint8_t before, uint8_t after) {
  if (before == 1 && after == 0) {
    m_tima++;
    if (m_tima == 0) {
      m_cycles_til_tima_irq = 1;
    }
  }
}

}  // namespace gbcxx
