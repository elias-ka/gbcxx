#include "core/timer.hpp"

#include "core/constants.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gbcxx {
auto Timer::read(uint16_t address) const -> uint8_t
{
    switch (address) {
    case Io_Reg::div: return m_div >> 8;
    case Io_Reg::tima: return m_tima;
    case Io_Reg::tma: return m_tma;
    case Io_Reg::tac: return m_tac & 0b111;
    }
    LOG_WARN("Timer: read from unhandled address {:#06x}", address);
    return 0;
}

auto Timer::write(uint16_t address, uint8_t value) -> void
{
    switch (address) {
    case Io_Reg::div: m_div = 0; break;
    case Io_Reg::tima: m_tima = value; break;
    case Io_Reg::tma: m_tma = value; break;
    case Io_Reg::tac: m_tac = value & 0b111; break;
    }
}

auto Timer::tick() -> void
{
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
                m_interrupts |= std::to_underlying(Interrupt::timer);
            }
        }
    }
}
} // namespace gbcxx
