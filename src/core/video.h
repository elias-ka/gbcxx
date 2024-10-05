#pragma once

#include "core/constants.h"
#include "util.h"
#include <array>

namespace cb
{
    class mmu;

    class frame_buffer
    {
    public:
        constexpr rgba pixel_color(usz x, usz y) const { return m_buf.at(index(x, y)); }
        constexpr void set_pixel_color(usz x, usz y, rgba color) { m_buf.at(index(x, y)) = color; }
        constexpr void fill(rgba color) { m_buf.fill(color); }

    private:
        usz index(usz x, usz y) const { return y * screen_width + x; }

        std::array<rgba, screen_width * screen_height> m_buf;
    };

    struct lcd_control
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

    struct lcd_status
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
        explicit ppu(mmu* mmu)
            : m_mmu(mmu)
        {
        }

        void tick();
        const frame_buffer& buffer() const { return m_buffer; }
        bool should_redraw() const { return m_should_redraw; }
        void clear_should_redraw() { m_should_redraw = false; }

    private:
        void enter_mode(mode mode);
        usz mode_cycles(mode mode) const;
        void increment_ly();
        void render_bg_scanline();

    private:
        frame_buffer m_buffer{};
        mode m_mode{mode::hblank};
        usz m_cycles{};
        bool m_should_redraw{};
        mmu* m_mmu{};
    };
} // namespace cb
