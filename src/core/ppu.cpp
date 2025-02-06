#include "core/ppu.hpp"

#include <algorithm>

#include "core/constants.hpp"
#include "core/core.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gb
{
void Ppu::Tick()
{
    if (!(lcdc_ & ControlBit::LcdDisplay))
    {
        return;
    }

    dots_++;

    switch (mode_)
    {
        case Mode::HBlank: OnHBlank(); break;
        case Mode::VBlank: OnVBlank(); break;
        case Mode::OamScan: OnOamScan(); break;
        case Mode::Transfer: OnTransfer(); break;
    }
}

void Ppu::SwitchMode(Mode new_mode)
{
    mode_ = new_mode;
    stat_ = (stat_ & 0xfc) | std::to_underlying(new_mode);

    const auto check_and_set_interrupt = [&](bool cond) -> void
    {
        if (cond)
        {
            gb_.Irq(Interrupt::Stat);
            LOG_DEBUG("PPU: stat irq");
        }
    };

    switch (new_mode)
    {
        case Mode::HBlank: check_and_set_interrupt(stat_ & StatusBit::Mode0Condition); break;
        case Mode::VBlank: check_and_set_interrupt(stat_ & StatusBit::Mode1Condition); break;
        case Mode::OamScan: check_and_set_interrupt(stat_ & StatusBit::Mode2Condition); break;
        case Mode::Transfer: break;
    }
}

uint8_t Ppu::Read(uint16_t addr) const
{
    if (kVramStart <= addr && addr <= kVramEnd)
    {
        return vram_.at(addr - kVramStart);
    }

    if (kOamStart <= addr && addr <= kOamEnd)
    {
        return oam_.at(addr - kOamStart);
    }

    switch (addr)
    {
        case kRegLcdc: return lcdc_;
        case kRegStat: return (stat_ & ~0x3) | std::to_underlying(mode_);
        case kRegScy: return scy_;
        case kRegScx: return scx_;
        case kRegLy: return ly_;
        case kRegLyc: return lyc_;
        case kRegBgp: return bgp_;
        case kRegObp0: return obp0_;
        case kRegObp1: return obp1_;
        case kRegWy: return wy_;
        case kRegWx: return wx_;
        default:
        {
            LOG_ERROR("PPU: Unmapped read {:X}", addr);
            return 0;
        }
    }
}

void Ppu::Write(uint16_t addr, uint8_t val)
{
    if (kVramStart <= addr && addr <= kVramEnd)
    {
        vram_.at(addr - kVramStart) = val;
        return;
    }

    if (kOamStart <= addr && addr <= kOamEnd)
    {
        oam_.at(addr - kOamStart) = val;
        return;
    }

    switch (addr)
    {
        case kRegLcdc: lcdc_ = val; break;
        case kRegStat: stat_ = val & ~0x3; break;
        case kRegScy: scy_ = val; break;
        case kRegScx: scx_ = val; break;
        case kRegLy: ly_ = val; break;
        case kRegLyc: lyc_ = val; break;
        case kRegBgp: bgp_ = val; break;
        case kRegObp0: obp0_ = val; break;
        case kRegObp1: obp1_ = val; break;
        case kRegWy: wy_ = val; break;
        case kRegWx: wx_ = val; break;
        default:
        {
            LOG_ERROR("PPU: Unmapped write {:X} <- {:X}", addr, val);
        }
    }
}

void Ppu::OnHBlank()
{
    if (dots_ >= kDotsHBlank)
    {
        dots_ -= kDotsHBlank;
        ly_ = (ly_ + 1) % 154;

        if (ly_ == 144)
        {
            SwitchMode(Mode::VBlank);
            frame_ready_ = true;
            gb_.Irq(Interrupt::VBlank);
        }
        else
        {
            SwitchMode(Mode::OamScan);
        }
    }
}

void Ppu::OnVBlank()
{
    if (dots_ >= kDotsVBlank)
    {
        dots_ -= kDotsVBlank;
        ly_ = (ly_ + 1) % 154;

        if (ly_ == lyc_)
        {
            stat_ |= StatusBit::LycEqLyInterrupt;
            if (stat_ & StatusBit::LycEqLyEnable)
            {
                gb_.Irq(Interrupt::Stat);
            }
        }

        if (ly_ == 153)
        {
            SwitchMode(Mode::OamScan);
            ly_ = 0;
        }
    }
}

void Ppu::OnOamScan()
{
    if (dots_ >= kDotsOamScan)
    {
        dots_ -= kDotsOamScan;
        SwitchMode(Mode::Transfer);
    }
}

void Ppu::OnTransfer()
{
    if (dots_ >= kDotsTransfer)
    {
        dots_ -= kDotsTransfer;
        SwitchMode(Mode::HBlank);
        RenderScanline();
    }
}

void Ppu::RenderScanline()
{
    Scanline scanline{};
    RenderBackgroundScanline(scanline);
    scanline_draw_callback_(scanline, ly_);
}

void Ppu::RenderBackgroundScanline(Scanline& scanline) const
{
    if (!(lcdc_ & ControlBit::BgAndWinDisplay))
    {
        std::ranges::fill(scanline, Color{0xff, 0xff, 0xff});
        return;
    }

    for (uint8_t lx = 0; lx < kLcdWidth; lx++)
    {
        const uint8_t x = lx + scx_;
        const uint8_t y = ly_ + scy_;

        uint16_t address = ((lcdc_ & ControlBit::BgTileMap) ? 0x1C00 : 0x1800) +
                           (((y / 8) % 32) * 32) + ((x / 8) % 32);
        const uint8_t tile_id = vram_.at(address);
        if (lcdc_ & ControlBit::BgAndWinTileData)
        {
            address = static_cast<uint16_t>((tile_id * 16) + ((y % 8) * 2));
        }
        else
        {
            address =
                static_cast<uint16_t>((static_cast<int8_t>(tile_id) * 16) + ((y % 8) * 2) + 0x1000);
        }
        const auto mask = static_cast<uint8_t>(1 << (7 - (x & 7)));
        const uint8_t byte1 = vram_.at(address);
        const uint8_t byte2 = vram_.at(address + 1);
        const auto color =
            static_cast<uint8_t>(((byte1 & mask) ? 1 : 0) + ((byte2 & mask) ? 2 : 0));
        scanline[x] = bg_palette_.at(color);
    }
}

}  // namespace gb
