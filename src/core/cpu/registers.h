#pragma once

#include "core/cpu/flags.h"
#include <cstdint>

namespace core::cpu
{
    enum class reg8
    {
        a,
        b,
        c,
        d,
        e,
        h,
        l
    };

    enum class reg16
    {
        af,
        bc,
        de,
        hl,
    };

    struct registers
    {
        uint16_t pc;
        uint16_t sp;
        uint8_t a;
        flags f;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t h;
        uint8_t l;

        [[nodiscard]] uint16_t read16(reg16 reg) const;
        void write16(reg16 reg, uint16_t value);
    };

} // namespace core::cpu
