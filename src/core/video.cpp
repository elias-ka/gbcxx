#include "core/video.h"

#include "constants.hpp"
#include "util.h"

namespace cb::video
{
    namespace
    {
        constexpr std::array<color::rgba, 4> dmg_palette = {
            color::white, {0xA9, 0xA9, 0xA9, 0xFF}, {0x54, 0x54, 0x54, 0xFF}, color::black};
    } // namespace

    void ppu::tick()
    {
        m_cycles += 1;
        switch (m_mode)
        {
        case mode::hblank:
        {
            m_line_y += 1;
            if (m_line_y == 144)
            {
                m_should_redraw = true;
                enter_mode(mode::vblank);
            }
            else
            {
                enter_mode(mode::oam_scan);
            }
            break;
        }
        case mode::vblank:
        {
            m_line_y += 1;
            if (m_line_y == 154)
            {
                m_line_y = 0;
                enter_mode(mode::oam_scan);
            }
            else
            {
                m_cycles += cycles_vblank;
            }
            break;
        }
        case mode::oam_scan:
        {
            enter_mode(mode::transfer);
            break;
        }
        case mode::transfer:
        {
            render_bg_scanline();
            enter_mode(mode::hblank);
            break;
        }
        }
    }

    u8 ppu::read8(u16 addr) const
    {
        if (0x8000 <= addr && addr <= 0x9FFF)
        {
            return m_vram.at(addr - 0x8000);
        }

        switch (addr)
        {
        case reg::lcdc: return m_control.value;
        case reg::stat: return m_stat.value;
        case reg::ly: return m_line_y;
        case reg::lyc: return m_line_y_compare;
        case reg::scy: return m_scroll_y;
        case reg::scx: return m_scroll_x;
        case reg::bgp: return m_bg_palette;
        default:
        {
            LOG_UNIMPLEMENTED("ppu read from {:#02x}", addr);
            return 0xFF;
        }
        }
    }

    void ppu::write8(u16 addr, u8 data)
    {
        if (0x8000 <= addr && addr <= 0x9FFF)
        {
            m_vram.at(addr - 0x8000) = data;
        }

        switch (addr)
        {
        case reg::lcdc: m_control.value = data; break;
        case reg::stat: m_stat.value = data; break;
        case reg::ly: m_line_y = data; break;
        case reg::lyc: m_line_y_compare = data; break;
        case reg::scy: m_scroll_y = data; break;
        case reg::scx: m_scroll_x = data; break;
        case reg::bgp: m_bg_palette = data; break;
        default: LOG_UNIMPLEMENTED("ppu write to {:#04x} with data {:#02x}", addr, data);
        }
    }

    void ppu::enter_mode(mode mode)
    {
        m_mode = mode;
        m_cycles += mode_cycles(mode);
    }

    void ppu::render_bg_scanline()
    {
        if (!m_control.bg_on())
            return;

        const u16 y = m_line_y + m_scroll_y;
        const u16 map_mask = m_control.bg_map() ? 0x1C00 : 0x1700;
        const usz row = y / 8;

        for (u8 i = 0; i < screen_width; i++)
        {
            const u8 x = i + m_scroll_x;
            const usz col = x / 8;

            usz tile_num = m_vram.at(((row * 32 + col) | map_mask) & 0x1FFF);
            if (m_control.bg_addr())
                tile_num = static_cast<usz>(128 + s16(s8(tile_num)) + 128);

            const usz line = (y % 8) * 2;
            const usz tile_mask = tile_num << 4;
            const u8 data1 = m_vram.at((tile_mask | line) & 0x1FFF);
            const u8 data2 = m_vram.at((tile_mask | line + 1) & 0x1FFF);
            const auto bit = static_cast<usz>(((x % 8) + 7) * 0xFF);
            const u8 color_value = (data2 & (1 << bit) << 1) | (data1 & (1 << bit));
            m_buffer.set_pixel_color(x, y, dmg_palette.at(color_value));
        }
    }

} // namespace cb::video
