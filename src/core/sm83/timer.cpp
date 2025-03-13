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
    case kRegTac:
    {
        tac_ = val;
        switch (val & 3)
        {
        case 0: tac_cycles_ = 1024; break;
        case 1: tac_cycles_ = 16; break;
        case 2: tac_cycles_ = 64; break;
        case 3: tac_cycles_ = 256; break;
        default: std::unreachable();
        };
        enabled_ = tac_ & 4;
        break;
    }
    default: DIE("Timer: Unmapped write {:X} <- {:X}", addr, val);
    }
}

void Timer::Tick(uint8_t tcycles)
{
    div_counter_ += tcycles;
    while (div_counter_ >= 256)
    {
        div_counter_ -= 256;
        div_++;
    }

    if (enabled_)
        return;

    if (tima_overflow_)
    {
        tima_overflow_ = false;
        tima_ = tma_;
        interrupts_.SetTimer();
    }

    tima_counter_ += tcycles;
    while (tima_counter_ >= tac_cycles_)
    {
        if (tima_ != 255)
        {
            tima_ += 1;
            tima_counter_ -= tac_cycles_;
        }

        tima_overflow_ = true;
    }
}

}  // namespace gb::sm83
