#pragma once

#include <cstddef>
#include <cstdint>

namespace gbcxx {
class Timer {
public:
    [[nodiscard]] auto read(uint16_t address) const -> uint8_t;
    void write(uint16_t address, uint8_t value);

    auto tick() -> void;
    auto get_and_clear_interrupts() -> uint8_t
    {
        const uint8_t interrupts = m_interrupts;
        m_interrupts = 0;
        return interrupts;
    }

private:
    size_t m_internal_div {};
    size_t m_internal_tima {};
    uint8_t m_div {};
    uint8_t m_tima {};
    uint8_t m_tma {};
    uint8_t m_tac {};
    uint8_t m_interrupts {};
};
} // namespace gbcxx
