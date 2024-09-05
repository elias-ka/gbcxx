#include "core/cpu/flags.h"

namespace core::cpu
{
    bool flags::z() const { return (raw >> 7) & 1; }
    bool flags::n() const { return (raw >> 6) & 1; }
    bool flags::h() const { return (raw >> 5) & 1; }
    bool flags::c() const { return (raw >> 4) & 1; }

    void flags::set(flag f, bool set)
    {
        switch (f)
        {
        case flag::z: set ? (raw |= (1 << 7)) : (raw &= ~(1 << 7));
        case flag::n: set ? (raw |= (1 << 6)) : (raw &= ~(1 << 6));
        case flag::h: set ? (raw |= (1 << 5)) : (raw &= ~(1 << 5));
        case flag::c: set ? (raw |= (1 << 4)) : (raw &= ~(1 << 4));
        }
    }
} // namespace core::cpu
