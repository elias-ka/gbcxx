#include "core/cpu/flags.h"

namespace core::cpu
{
    namespace
    {
        constexpr uint8_t z_mask = 1 << 7;
        constexpr uint8_t n_mask = 1 << 6;
        constexpr uint8_t h_mask = 1 << 5;
        constexpr uint8_t c_mask = 1 << 4;
    } // namespace

    bool flags::z() const { return raw & z_mask; }
    bool flags::n() const { return raw & n_mask; }
    bool flags::h() const { return raw & h_mask; }
    bool flags::c() const { return raw & c_mask; }

    void flags::set(flag f, bool set)
    {
        switch (f)
        {
        case flag::z: set ? (raw |= z_mask) : (raw &= ~z_mask);
        case flag::n: set ? (raw |= n_mask) : (raw &= ~n_mask);
        case flag::h: set ? (raw |= h_mask) : (raw &= ~h_mask);
        case flag::c: set ? (raw |= c_mask) : (raw &= ~c_mask);
        }
    }
} // namespace core::cpu
