#pragma once

#include <cstdint>

namespace core::cpu
{
    enum class flag
    {
        z,
        n,
        h,
        c
    };

    struct flags
    {
        uint8_t raw;

        [[nodiscard]] bool z() const;
        [[nodiscard]] bool n() const;
        [[nodiscard]] bool h() const;
        [[nodiscard]] bool c() const;

        void set(flag f, bool set = true);
    };

} // namespace core::cpu
