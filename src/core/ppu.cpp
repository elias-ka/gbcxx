#include "core/ppu.hpp"

#include "core/constants.hpp"
#include "core/core.hpp"
#include "core/interrupt.hpp"
#include "util.hpp"

namespace gb
{
void Ppu::Step(int cycles)
{
    if (!(lcdc_ & ControlBit::LcdDisplay))
        return;

    dots_ += cycles;

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

    if (new_mode == Mode::VBlank)
        gb_.Irq(Interrupt::VBlank);
}

void Ppu::CheckLycInterrupt()
{
    if (lyc_ == ly_)
        stat_ |= StatusBit::LycEqLyInterrupt;
    else
        stat_ &= ~StatusBit::LycEqLyInterrupt;
}

void Ppu::CheckStatInterrupt(bool on_vblank)
{
    if ((mode_ == Mode::HBlank && (stat_ & StatusBit::Mode0Condition)) ||
        (mode_ == Mode::VBlank && (stat_ & StatusBit::Mode1Condition)) ||
        (mode_ == Mode::OamScan && (stat_ & StatusBit::Mode2Condition)) ||
        ((lcdc_ & StatusBit::LycEqLyEnable) &&
         (lcdc_ & StatusBit::LycEqLyInterrupt)) ||
        (on_vblank && (stat_ & StatusBit::Mode2Condition)))
    {
        gb_.Irq(Interrupt::Lcd);
    }
}

uint8_t Ppu::Read(uint16_t addr) const
{
    if (kVramStart <= addr && addr <= kVramEnd)
    {
        if (!CanAccessVramAndOam())
            return 0xff;

        return vram_[addr - kVramStart];
    }

    if (kOamStart <= addr && addr <= kOamEnd)
    {
        if (!CanAccessVramAndOam())
            return 0xff;

        return oam_[addr - kOamStart];
    }

    switch (addr)
    {
    case kRegLcdc: return lcdc_;
    case kRegStat: return (stat_ & ~0x3) | std::to_underlying(mode_);
    case kRegScy: return scy_;
    case kRegScx: return scx_;
    // case kRegLy: return ly_;
    case kRegLy: return 0x90;
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
    // LOG_TRACE("Ppu::Write({:X}, {:X})", addr, val);

    if (kVramStart <= addr && addr <= kVramEnd)
    {
        if (CanAccessVramAndOam())
            vram_[addr - kVramStart] = val;

        return;
    }

    if (kOamStart <= addr && addr <= kOamEnd)
    {
        if (CanAccessVramAndOam())
            oam_[addr - kOamStart] = val;

        return;
    }

    switch (addr)
    {
    case kRegLcdc: lcdc_ = val; break;
    case kRegStat: stat_ = val & ~0x3; break;
    case kRegScy: scy_ = val; break;
    case kRegScx: scx_ = val; break;
    case kRegLy: ly_ = 0; break;
    case kRegLyc: lyc_ = val; break;
    case kRegBgp: bgp_ = val; break;
    case kRegObp0: obp0_ = val; break;
    case kRegObp1: obp1_ = val; break;
    case kRegWy: wy_ = val; break;
    case kRegWx: wx_ = val; break;
    default: LOG_ERROR("PPU: Unmapped write {:X} <- {:X}", addr, val);
    }
}

void Ppu::OnHBlank()
{
    const int dots_hblank = [this]
    {
        switch (scx_ & 0x7)
        {
        case 0: return kDotsHBlank;
        case 1:
        case 2:
        case 3:
        case 4: return 200;
        case 5:
        case 6:
        case 7: return 196;
        default: std::unreachable();
        }
    }();

    if (dots_ >= dots_hblank)
    {
        dots_ -= dots_hblank;
        ly_++;

        CheckLycInterrupt();

        if (ly_ == 144)
        {
            SwitchMode(Mode::VBlank);
            CheckStatInterrupt(true);
            vblank_callback_(fb_);
        }
        else
        {
            SwitchMode(Mode::OamScan);
            CheckStatInterrupt();
        }
    }
}

void Ppu::OnVBlank()
{
    if (dots_ >= kDotsVBlank)
    {
        dots_ -= kDotsVBlank;
        ly_++;

        CheckLycInterrupt();
        CheckStatInterrupt();

        if (ly_ == 154)
        {
            SwitchMode(Mode::OamScan);
            CheckLycInterrupt();
            CheckStatInterrupt();
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
        CheckStatInterrupt();
        RenderScanline();
    }
}

void Ppu::RenderScanline()
{
    RenderBackgroundScanline();
}

void Ppu::RenderBackgroundScanline()
{
    if (!(lcdc_ & ControlBit::BgAndWinDisplay))
    {
        fb_.FillScanline(ly_, {0xff, 0xff, 0xff});
        return;
    }

    const uint16_t y = ly_ + scy_;
    const uint8_t tile_row = (y >> 3) & 31;
    const auto pixel_row_offset = static_cast<uint8_t>((y & 7) << 1);
    const uint16_t tile_map_base = GetTileMapBase();

    for (uint8_t lx = 0; lx < kLcdWidth; ++lx)
    {
        const uint16_t x = lx + scx_;
        const uint8_t tile_col = (x >> 3) & 31;
        const uint8_t pixel_col_offset = 7 - (x & 7);
        const uint8_t tile_id = vram_[static_cast<size_t>(
            tile_map_base | (tile_row << 5) | tile_col)];

        const size_t address =
            (lcdc_ & ControlBit::BgAndWinTileData)
                ? static_cast<size_t>(tile_id << 4) | pixel_row_offset
                : static_cast<size_t>((static_cast<int8_t>(tile_id) << 4) |
                                      pixel_row_offset | 0x1000);

        const uint8_t byte1 = vram_[address];
        const uint8_t byte2 = vram_[address + 1];

        const auto mask = 1 << pixel_col_offset;
        uint8_t coloridx = ((byte1 & mask) ? 1 : 0) | ((byte2 & mask) ? 2 : 0);
        coloridx = (bgp_ >> (coloridx << 1)) & 3;

        fb_.SetPixelColor(lx, ly_, Color::FromIndex(coloridx));
    }
}

void Ppu::DrawTileMap(std::span<gb::Color> buf, uint16_t tile_address) const
{
    constexpr size_t kMapSize = 32;
    constexpr size_t kTileSize = 8;
    constexpr size_t kMapWidth = kMapSize * kTileSize;

    assert(buf.size() == (kMapWidth * kMapWidth));

    for (size_t px = 0; px < (kMapWidth * kMapWidth); ++px)
    {
        const size_t x_pos = px % kMapWidth;
        const size_t y_pos = px / kMapWidth;

        const size_t tile_x = x_pos / kTileSize;
        const size_t tile_y = y_pos / kTileSize;

        const uint8_t tile_id =
            vram_[tile_address + (tile_y * kMapSize) + tile_x];
        const auto tile_base = static_cast<size_t>(tile_id) * 16;

        const size_t tile_row = y_pos % kTileSize;
        const size_t tile_col = x_pos % kTileSize;

        const uint8_t byte1 = vram_[tile_base + (tile_row * 2)];
        const uint8_t byte2 = vram_[tile_base + (tile_row * 2) + 1];

        const auto mask = static_cast<uint8_t>(1 << (7 - tile_col));
        uint8_t color_idx = ((byte1 & mask) ? 1 : 0) | ((byte2 & mask) ? 2 : 0);
        color_idx = (bgp_ >> (color_idx << 1)) & 3;

        buf[px] = Color::FromIndex(color_idx);
    }
}
}  // namespace gb
