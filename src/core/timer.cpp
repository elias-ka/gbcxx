#include "core/timer.hpp"

#include "core/constants.hpp"
#include "core/core.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gb
{
uint8_t Timer::Read(uint16_t addr) const
{
    // LOG_TRACE("Timer::read({:X})", addr);
    switch (addr)
    {
    case kRegDiv: return (sysclk_ >> 6) & 0xff;
    case kRegTima: return tima_;
    case kRegTma: return tma_;
    case kRegTac: return tac_;
    default:
    {
        DIE("Timer: read from unhandled address {:X}", addr);
    }
    }
}

void Timer::Write(uint16_t addr, uint8_t val)
{
    // LOG_TRACE("Timer::write({:X}, {:#b})", addr, val);
    switch (addr)
    {
    case kRegDiv:
    {
        SysclkWrite(0);
        break;
    }
    case kRegTima:
    {
        if (!tima_reload_cycle_)
        {
            tima_ = val;
        }
        if (cycles_til_tima_irq_ == 1)
        {
            cycles_til_tima_irq_ = 0;
        }
        break;
    }
    case kRegTma:
    {
        if (tima_reload_cycle_)
        {
            tima_ = val;
        }

        tma_ = val;
        break;
    }
    case kRegTac:
    {
        const uint8_t prev_bit = last_bit_;
        last_bit_ &= ((val & 0x4) >> 2);
        DetectEdge(prev_bit, last_bit_);
        tac_ = val;
        break;
    }
    default:
    {
        DIE("Timer: Unmapped write {:X} <- {:X}", addr, val);
    }
    }
}

void Timer::Tick()
{
    tima_reload_cycle_ = false;

    if (cycles_til_tima_irq_ > 0)
    {
        // LOG_TRACE("cycles_til_tima_irq: {}", cycles_til_tima_irq_);
        if (--cycles_til_tima_irq_ == 0)
        {
            gb_.Irq(Interrupt::Timer);
            tima_ = tma_;
            tima_reload_cycle_ = true;
        }
    }
    SysclkWrite(sysclk_ + 1);
}

void Timer::SysclkWrite(uint8_t new_val)
{
    sysclk_ = new_val;

    auto bit = [this] -> uint8_t
    {
        switch (tac_ & 0x3)
        {
        case 3: return (sysclk_ >> 5) & 1;
        case 2: return (sysclk_ >> 3) & 1;
        case 1: return (sysclk_ >> 1) & 1;
        case 0: return (sysclk_ >> 7) & 1;
        default: std::unreachable();
        }
    }();

    bit &= (tac_ & 0x4) >> 2;

    DetectEdge(last_bit_, bit);
    last_bit_ = bit;
}

void Timer::DetectEdge(uint8_t before, uint8_t after)
{
    if (before == 1 && after == 0)
    {
        tima_++;

        if (tima_ == 0)
        {
            cycles_til_tima_irq_ = 1;
        }
    }
}

}  // namespace gb
