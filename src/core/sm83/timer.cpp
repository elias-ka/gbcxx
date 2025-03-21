#include "core/sm83/timer.hpp"

#include "core/constants.hpp"
#include "core/sm83/interrupts.hpp"
#include "core/util.hpp"

namespace gb::sm83
{
uint8_t Timer::ReadByte(uint16_t addr) const
{
    switch (addr)
    {
    case kRegDiv: return div_;
    case kRegTima: return tima_;
    case kRegTma: return tma_;
    case kRegTac: return tac_;
    default: DIE("Timer: Unmapped read {:X}", addr);
    }
}

void Timer::WriteByte(uint16_t addr, uint8_t val)
{
    switch (addr)
    {
    case kRegDiv: div_ = 0; break;
    case kRegTima: tima_ = val; break;
    case kRegTma: tma_ = val; break;
    case kRegTac: tac_ = val; break;
    default: DIE("Timer: Unmapped write {:X} <- {:X}", addr, val);
    }
}

void Timer::Tick(uint8_t tcycles)
{
    // ref: https://markau.dev/posts/time-for-timers/
    internal_div_ += tcycles;
    while (internal_div_ >= 256)
    {
        internal_div_ -= 256;
        div_++;
    }

    if (!(tac_ & 4)) return;

    int input_clock;
    switch (tac_ & 3)
    {
    case 0: input_clock = 1024; break;
    case 1: input_clock = 16; break;
    case 2: input_clock = 64; break;
    case 3: input_clock = 256; break;
    default: std::unreachable();
    }

    internal_tima_ += tcycles;
    while (internal_tima_ >= input_clock)
    {
        tima_++;
        internal_tima_ -= input_clock;
        if (tima_ == 0)
        {
            tima_ = tma_;
            interrupts_ |= sm83::IntTimer;
        }
    }
}

}  // namespace gb::sm83
