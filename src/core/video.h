#pragma once

#include "core/constants.hpp"
#include "util.h"
#include <array>

namespace cb::video
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

        bool bg_on() const { return value & (1 << 0); }
        bool obj_on() const { return value & (1 << 1); }
        bool obj_size() const { return value & (1 << 2); }
        bool bg_map() const { return value & (1 << 3); }
        bool bg_addr() const { return value & (1 << 4); }
        bool window_on() const { return value & (1 << 5); }
        bool window_map() const { return value & (1 << 6); }
        bool lcd_on() const { return value & (1 << 7); }
    };

    struct stat
    {
        u8 value;

        bool compare() const { return value & (1 << 2); }
        bool int_hblank() const { return value & (1 << 3); }
        bool int_vblank() const { return value & (1 << 4); }
        bool int_access_oam() const { return value & (1 << 5); }
        bool int_compare() const { return value & (1 << 6); }
    };

    enum class mode
    {
        hblank,
        vblank,
        oam_scan,
        transfer
    };

    inline usz mode_cycles(mode mode)
    {
        switch (mode)
        {
        case mode::hblank: return cycles_hblank;
        case mode::vblank: return cycles_vblank;
        case mode::oam_scan: return cycles_oam_scan;
        case mode::transfer: return cycles_transfer;
        }
    }

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
        void render_bg_scanline();

    private:
        frame_buffer m_buffer{};
        std::array<u8, 8_KiB> m_vram{};
        mode m_mode{mode::hblank};
        usz m_cycles{};
        control m_control{};
        stat m_stat{};
        bool m_should_redraw{};
        u8 m_vram_bank{};
        u8 m_scroll_y{};
        u8 m_scroll_x{};
        u8 m_line_y{};
        u8 m_line_y_compare{};
        u8 m_bg_palette{};
    };
} // namespace cb::video
