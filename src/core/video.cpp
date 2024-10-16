#include "core/video.h"

#include "core/constants.h"
#include "util.h"
#include <cassert>

namespace cb
{
    namespace
    {
        constexpr std::array<Rgba, 4> dmg_palette = {{{0xFF, 0xFF, 0xFF, 0xFF},
                                                      {0xB0, 0xB0, 0xB0, 0xFF},
                                                      {0x68, 0x68, 0x68, 0xFF},
                                                      {0x00, 0x00, 0x00, 0xFF}}};
    } // namespace

    u8 Ppu::read(u16 addr) const
    {
        if (VRAM_START <= addr && addr <= VRAM_END)
            return m_vram.at(addr - VRAM_START);

        if (OAM_START <= addr && addr <= OAM_END)
            return m_oam.at(addr - OAM_START);

        switch (addr)
        {
        case REG_LCDC: return m_lcdc.raw;
        case REG_STAT: return m_stat.raw;
        case REG_SCY: return m_scy;
        case REG_SCX: return m_scx;
        case REG_LY: return m_ly;
        case REG_LYC: return m_lyc;
        case REG_BGP: return m_bgp;
        case REG_OBP0: return m_obp0;
        case REG_OBP1: return m_obp1;
        case REG_WY: return m_wy;
        case REG_WX: return m_wx;
        default: return 0;
        }
    }

    void Ppu::write(u16 addr, u8 data)
    {
        if (VRAM_START <= addr && addr <= VRAM_END)
        {
            m_vram.at(addr - VRAM_START) = data;
            return;
        }

        if (OAM_START <= addr && addr <= OAM_END)
        {
            m_oam.at(addr - OAM_START) = data;
            return;
        }

        switch (addr)
        {
        case REG_LCDC: m_lcdc.raw = data; break;
        case REG_STAT:
        {
            m_stat.raw = data & 0xF8;
            break;
        }
        case REG_SCY: m_scy = data; break;
        case REG_SCX: m_scx = data; break;
        case REG_LY: m_ly = data; break;
        case REG_BGP: m_bgp = data; break;
        case REG_OBP0: m_obp0 = data; break;
        case REG_OBP1: m_obp1 = data; break;
        case REG_WY: m_wy = data; break;
        case REG_WX: m_wx = data; break;
        // read-only
        case REG_LYC: break;
        }
    }

    void Ppu::tick()
    {
        m_cycles++;
        switch (m_mode)
        {
        case Mode::hblank:
        {
            if (m_cycles == mode_cycles(Mode::hblank))
            {
                m_cycles = 0;
                m_ly++;
                if (m_ly == 144)
                {
                    enter_mode(Mode::vblank);
                    m_should_redraw = true;
                }
                else
                {
                    enter_mode(Mode::oam_scan);
                }
            }
            break;
        }
        case Mode::vblank:
        {
            if (m_cycles == mode_cycles(Mode::vblank))
            {
                m_cycles = 0;
                m_ly++;
                if (m_ly == 154)
                {
                    enter_mode(Mode::oam_scan);
                    m_ly = 0;
                }
            }
            break;
        }
        case Mode::oam_scan:
        {
            if (m_cycles == mode_cycles(Mode::oam_scan))
            {
                m_cycles = 0;
                enter_mode(Mode::transfer);
            }
            break;
        }
        case Mode::transfer:
        {
            if (m_cycles == mode_cycles(Mode::transfer))
            {
                m_cycles = 0;
                enter_mode(Mode::hblank);
                render_bg_scanline();
            }
            break;
        }
        }
    }

    void Ppu::enter_mode(Mode mode) { m_mode = mode; }

    usz Ppu::mode_cycles(Mode mode) const
    {
        switch (mode)
        {
        case Mode::hblank:
        {
            switch (m_scx % 8)
            {
            case 0: return CYCLES_HBLANK;
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
        case Mode::vblank: return CYCLES_VBLANK;
        case Mode::oam_scan: return CYCLES_OAM_SCAN;
        case Mode::transfer: return CYCLES_TRANSFER;
        }
    }

    void Ppu::render_bg_scanline()
    {
        if (!m_lcdc.bg_on())
            return;

        const u8 scrolled_y = m_ly + m_scy;
        for (u8 lx = 0; lx < SCREEN_WIDTH; lx++)
        {
            const u8 scrolled_x = lx + m_scx;
            m_buffer.set_pixel_color(lx, m_ly, bg_color(scrolled_x, scrolled_y));
        }
    }

    Rgba Ppu::bg_color(u8 x, u8 y) const
    {
        u16 address = (m_lcdc.bg_map() ? 0x1C00 : 0x1800) + ((y >> 3) & 31) * 32 + ((x >> 3) & 31);
        const u8 tile_id = m_vram.at(address);
        if (m_lcdc.bg_addr())
            address = static_cast<u16>((tile_id << 4) + ((y & 7) << 1));
        else
            address = static_cast<u16>((static_cast<s8>(tile_id) << 4) + ((y & 7) << 1) + 0x1000);
        const u8 mask = static_cast<u8>(1 << (7 - (x & 7)));
        const u8 byte1 = m_vram.at(address);
        const u8 byte2 = m_vram.at(address + 1);
        const u8 color = static_cast<u8>(((byte1 & mask) ? 1 : 0) + ((byte2 & mask) ? 2 : 0));
        return dmg_palette.at(color);
    }

} // namespace cb
