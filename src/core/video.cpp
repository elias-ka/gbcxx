#include "core/video.h"

#include "core/constants.h"
#include "util.h"
#include <cassert>

namespace cb
{
    namespace
    {
        constexpr std::array<color::rgba, 4> dmg_palette = {
            color::black, {0xAA, 0xAA, 0xAA, 0xFF}, {0x55, 0x55, 0x55, 0xFF}, color::white};
    } // namespace

    void ppu::tick()
    {
        m_cycles += 1;
        // LOG_DEBUG("ppu cycles={}, mode={}", m_cycles, to_underlying(m_mode));
        switch (m_mode)
        {
        case mode::hblank:
        {
            if (m_cycles == mode_cycles(mode::hblank))
            {
                m_cycles = 0;
                m_line_y += 1;
                if (m_line_y == 144)
                {
                    enter_mode(mode::vblank);
                    m_should_redraw = true;
                }
                else
                {
                    enter_mode(mode::oam_scan);
                }
            }
            break;
        }
        case mode::vblank:
        {
            if (m_cycles == mode_cycles(mode::vblank))
            {
                m_cycles = 0;
                m_line_y += 1;
                if (m_line_y == 154)
                {
                    enter_mode(mode::oam_scan);
                    m_line_y = 0;
                }
            }
            break;
        }
        case mode::oam_scan:
        {
            if (m_cycles == mode_cycles(mode::oam_scan))
            {
                m_cycles = 0;
                enter_mode(mode::transfer);
            }
            break;
        }
        case mode::transfer:
        {
            if (m_cycles == mode_cycles(mode::transfer))
            {
                m_cycles = 0;
                enter_mode(mode::hblank);
                render_bg_scanline();
            }
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
        case reg_lcdc: return m_control.value;
        case reg_stat: return m_stat.value;
        case reg_ly: return m_line_y;
        case reg_lyc: return m_line_y_compare;
        case reg_scy: return m_scroll_y;
        case reg_scx: return m_scroll_x;
        case reg_bgp: return m_bg_palette;
        default:
        {
            LOG_UNIMPLEMENTED("ppu::read8({:#04x})", addr);
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
        case reg_lcdc: m_control.value = data; break;
        case reg_stat: m_stat.value = data; break;
        case reg_ly: m_line_y = data; break;
        case reg_lyc: m_line_y_compare = data; break;
        case reg_scy: m_scroll_y = data; break;
        case reg_scx: m_scroll_x = data; break;
        case reg_bgp: m_bg_palette = data; break;
        default: LOG_UNIMPLEMENTED("ppu::write8({:#04x}, {:#02x})", addr, data);
        }
    }

    void ppu::enter_mode(mode mode) { m_mode = mode; }

    usz ppu::mode_cycles(mode mode) const
    {
        switch (mode)
        {
        case mode::hblank:
        {
            switch (m_scroll_x % 8)
            {
            case 0: return 204;
            case 1:
            case 2:
            case 3:
            case 4: return 200;
            case 5:
            case 6:
            case 7: return 196;
            default: return 0;
            }
        };
        case mode::vblank: return cycles_vblank;
        case mode::oam_scan: return cycles_oam_scan;
        case mode::transfer: return cycles_transfer;
        }
    }

    void ppu::render_bg_scanline()
    {
        if (!m_control.bg_on())
            return;

        const u16 map_mask = m_control.bg_map() ? 0x1C00 : 0x1800;
        const u16 y = m_line_y + m_scroll_y;
        const usz row = y / 8;

        for (u8 i = 0; i < screen_width; i++)
        {
            const u8 x = i + m_scroll_x;
            const usz col = x / 8;

            usz tile_num = m_vram.at(((row * 32 + col) | map_mask) & 0x1FFF);
            if (!m_control.bg_addr())
            {
                tile_num =
                    static_cast<usz>(128 + static_cast<s16>(static_cast<s8>(tile_num)) + 128);
            }

            const usz line = static_cast<usz>(y % 8) * 2;
            const usz tile_mask = tile_num << 4;
            const u8 data1 = m_vram.at((tile_mask | line) & 0x1FFF);
            const u8 data2 = m_vram.at((tile_mask | (line + 1)) & 0x1FFF);
            // const auto bit = static_cast<usz>(((x % 8) + 7) * 0xFF);
            // const u8 color_value =
            //     static_cast<u8>(((data2 >> bit) & 1) << 1) | ((data1 >> bit) & 1);
            // m_buffer.set_pixel_color(x, y, dmg_palette.at(color_value));
        }
    }

} // namespace cb
