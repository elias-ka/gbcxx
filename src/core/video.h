#pragma once

#include "core/constants.h"
#include "util.h"
#include <array>

namespace cb
{
    class frame_buffer
    {
    public:
        constexpr color::rgba pixel_color(usz x, usz y) const { return m_buf.at(index(x, y)); }
        constexpr void set_pixel_color(usz x, usz y, color::rgba color)
        {
            m_buf.at(index(x, y)) = color;
        }
        constexpr void fill(color::rgba color) { m_buf.fill(color); }

    private:
        usz index(usz x, usz y) const { return y * screen_width + x; }

        std::array<color::rgba, screen_width * screen_height> m_buf;
    };

    struct control
    {
        u8 value;

        bool bg_on() const { return value & BIT(0); }
        bool obj_on() const { return value & BIT(1); }
        bool obj_size() const { return value & BIT(2); }
        bool bg_map() const { return value & BIT(3); }
        bool bg_addr() const { return value & BIT(4); }
        bool window_on() const { return value & BIT(5); }
        bool window_map() const { return value & BIT(6); }
        bool lcd_on() const { return value & BIT(7); }
    };

    struct stat
    {
        u8 value;

        bool compare() const { return value & BIT(2); }
        bool int_hblank() const { return value & BIT(3); }
        bool int_vblank() const { return value & BIT(4); }
        bool int_access_oam() const { return value & BIT(5); }
        bool int_compare() const { return value & BIT(6); }
    };

    enum class mode
    {
        hblank,
        vblank,
        oam_scan,
        transfer
    };

    class ppu
    {
    public:
        void tick();

        u8 read8(u16 addr) const;
        void write8(u16 addr, u8 data);

        frame_buffer& buffer() { return m_buffer; }
        bool should_redraw() const { return m_should_redraw; }
        void clear_should_redraw() { m_should_redraw = false; }

    private:
        void write_lcdc(u8 value);
        void enter_mode(mode mode);
        usz mode_cycles(mode mode) const;

        void render_bg_scanline();

    private:
        frame_buffer m_buffer{};
        std::array<u8, 8_KiB> m_vram{};
        mode m_mode{mode::hblank};
        usz m_cycles{};
        usz m_off_cycles{};
        control m_control{0x91};
        stat m_stat{0x85};
        bool m_should_redraw{};
        u8 m_scroll_y{0x00};
        u8 m_scroll_x{0x00};
        u8 m_line_y{0x00};
        u8 m_line_y_compare{0x00};
        u8 m_bg_palette{0xFC};
    };
} // namespace cb
