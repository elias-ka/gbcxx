#pragma once

#include <bitset>
#include <cstdint>

#include "core/util.hpp"

namespace gb
{
enum class Button : uint8_t
{
    A = 0,
    B,
    Select,
    Start,

    Right,
    Left,
    Up,
    Down,

    Count
};

class Joypad
{
public:
    [[nodiscard]] uint8_t ReadButtons() const
    {
        uint8_t buttons = 0xf;

        if (!select_buttons_ && !select_dpad_)
            return buttons;

        if (select_buttons_ && select_dpad_)
            LOG_WARN("Joypad: Both buttons and d-pad were selected (wtf)");

        const uint8_t offset = select_dpad_ ? 4 : 0;
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (button_states_.test(offset + i))
                buttons ^= (1 << (3 - i));
        }

        return buttons;
    }

    void Write(uint8_t val)
    {
        select_buttons_ = !(val & 0b0010'0000);
        select_dpad_ = !(val & 0b0001'0000);
    }

    void SetButton(Button btn, bool set) { button_states_[std::to_underlying(btn)] = set; }

private:
    bool select_buttons_{};
    bool select_dpad_{};
    std::bitset<static_cast<size_t>(Button::Count)> button_states_;
};
}  // namespace gb
