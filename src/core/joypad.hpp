#pragma once

#include <bitset>
#include <cstdint>
#include <utility>

#include "core/util.hpp"

namespace gb
{
enum class Input : uint8_t
{
    // Action buttons
    Start = 0,
    Select,
    B,
    A,

    // D-Pad directions
    Down,
    Up,
    Left,
    Right,

    Count
};

class Joypad
{
public:
    [[nodiscard]] uint8_t ReadButtons() const noexcept
    {
        uint8_t buttons = 0b0000'1111;

        if (!select_buttons_ && !select_dpad_) { return buttons; }

        if (select_buttons_ && select_dpad_) [[unlikely]]
        {
            LOG_WARN("Joypad: Both button groups selected simultaneously (wtf)");
            return buttons;
        }

        const size_t offset = select_dpad_ ? std::to_underlying(Input::Down) : 0;

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (button_states_[offset + i]) { buttons &= ~(1 << (3 - i)); }
        }

        return buttons;
    }

    void Write(uint8_t val) noexcept
    {
        select_buttons_ = !(val & 0b0010'0000);
        select_dpad_ = !(val & 0b0001'0000);
    }

    void SetButton(Input button, bool pressed) noexcept
    {
        button_states_[std::to_underlying(button)] = pressed;
    }

private:
    bool select_buttons_{};
    bool select_dpad_{};
    std::bitset<static_cast<size_t>(Input::Count)> button_states_;
};

}  // namespace gb
