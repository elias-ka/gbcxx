#include "core/cpu/registers.h"

namespace core::cpu
{
    uint16_t registers::read16(reg16 reg) const
    {
        switch (reg)
        {
        case reg16::af: return static_cast<uint16_t>(a << 8) | f.raw;
        case reg16::bc: return static_cast<uint16_t>(b << 8) | c;
        case reg16::de: return static_cast<uint16_t>(d << 8) | e;
        case reg16::hl: return static_cast<uint16_t>(h << 8) | l;
        }
    }

    void registers::write16(reg16 reg, uint16_t value)
    {
        switch (reg)
        {
        case reg16::af:
            a = static_cast<uint8_t>(value >> 8);
            f.raw = static_cast<uint8_t>(value & 0x00F0);
        case reg16::bc:
            b = static_cast<uint8_t>(value >> 8);
            c = static_cast<uint8_t>(value & 0x00FF);
        case reg16::de:
            d = static_cast<uint8_t>(value >> 8);
            e = static_cast<uint8_t>(value & 0x00FF);
        case reg16::hl:
            h = static_cast<uint8_t>(value >> 8);
            l = static_cast<uint8_t>(value & 0x00FF);
        }
    }
} // namespace core::cpu
