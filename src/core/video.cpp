#include "core/video.h"

#include "core/constants.h"
#include "core/memory.h"
#include "util.h"
#include <cassert>

namespace cb
{
    namespace
    {
        constexpr std::array<rgba, 4> dmg_palette = {{{0x7B, 0x82, 0x10, 0xFF},
                                                      {0x39, 0x59, 0x4A, 0xFF},
                                                      {0x55, 0x55, 0x55, 0xFF},
                                                      {0x5A, 0x79, 0x42, 0xFF}}};
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
                increment_ly();
                if (m_mmu->read8(reg_ly) == 144)
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
                increment_ly();
                if (m_mmu->read8(reg_ly) == 154)
                {
                    enter_mode(mode::oam_scan);
                    m_mmu->write8(reg_ly, 0);
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

    void ppu::enter_mode(mode mode) { m_mode = mode; }

    usz ppu::mode_cycles(mode mode) const
    {
        switch (mode)
        {
        case mode::hblank:
        {
            switch (m_mmu->read8(reg_scx) % 8)
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

    void ppu::increment_ly()
    {
        const u8 ly = m_mmu->read8(reg_ly);
        m_mmu->write8(reg_ly, ly + 1);
    }

    void ppu::render_bg_scanline()
    {
        const auto lcdc = m_mmu->read_as<lcd_control>(reg_lcdc);
        if (!lcdc.bg_on())
            return;

        const u8 screen_y = m_mmu->read8(reg_ly);
        const u8 scroll_y = m_mmu->read8(reg_scy);

        for (u8 screen_x = 0; screen_x < screen_width; screen_x++)
        {
            const u8 scroll_x = m_mmu->read8(reg_scx);
            const u8 tile_x = (scroll_x + screen_x) / 8;
            const u8 tile_y = (scroll_y + screen_y) / 8;
            const u16 tile_map_addr = 0x9800 + (tile_y * 32) + tile_x;
            const u8 tile_number = m_mmu->read8(tile_map_addr);
            const u16 tile_data_addr =
                lcdc.bg_addr() ? (0x8000 + tile_number * 16) :
                                 (0x8800 + static_cast<u16>(static_cast<s8>(tile_number) * 16));
            const u8 tile_line = (scroll_y + screen_y) % 8;
            const u8 tile_data1 = m_mmu->read8(tile_data_addr + tile_line * 2);
            const u8 tile_data2 = m_mmu->read8(tile_data_addr + tile_line * 2 + 1);
            const u8 bit_position = (scroll_x + screen_x) % 8;
            const u8 color_bit1 = (tile_data1 >> (7 - bit_position)) & 1;
            const u8 color_bit2 = (tile_data2 >> (7 - bit_position)) & 1;
            const u8 color_number = static_cast<u8>((color_bit2 << 1) | color_bit1);
            m_buffer.set_pixel_color(screen_x, screen_y, dmg_palette.at(color_number));
        }
    }

} // namespace cb
