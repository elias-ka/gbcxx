#pragma once

namespace cb
{
    enum class Interrupt
    {
        vblank = 0x00,
        lcd_stat = 0x01,
        timer = 0x02,
        serial = 0x03,
        joypad = 0x04
    };

} // namespace cb
