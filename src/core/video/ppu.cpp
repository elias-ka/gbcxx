#include "core/video/ppu.hpp"

#include <cstddef>

#include "core/constants.hpp"
#include "core/interrupt.hpp"
#include "core/util.hpp"

namespace gb
{
uint8_t Ppu::ReadByte(uint16_t addr) const
{
    if (addr >= kVramStart && addr <= kVramEnd)
        return vram_[addr - kVramStart];

    if (addr >= kOamStart && addr <= kOamEnd)
        return oam_[addr - kOamStart];

    switch (addr)
    {
    case kRegLcdc: return static_cast<uint8_t>(lcd_control_);
    case kRegStat: return static_cast<uint8_t>(lcd_status_);
    case kRegScy: return background_.scroll_y;
    case kRegScx: return background_.scroll_x;
    // case kRegLy: return ly_;
    case kRegLy: return 0x90;
    case kRegLyc: return lyc_;
    case kRegBgp: return bgp_;
    case kRegObp0: return obp0_;
    case kRegObp1: return obp1_;
    case kRegWy: return window_.GetY();
    case kRegWx: return window_.GetX();
    default: LOG_ERROR("PPU: Unmapped read {:X}", addr); return 0;
    }
}

void Ppu::WriteByte(uint16_t addr, uint8_t val)
{
    if (addr >= kVramStart && addr <= kVramEnd)
        vram_[addr - kVramStart] = val;
    else if (addr >= kOamStart && addr <= kOamEnd)
        oam_[addr - kOamStart] = val;
    else
        switch (addr)
        {
        case kRegLcdc: SetLcdc(val); break;
        case kRegStat: lcd_status_ = LcdStatus{val}; break;
        case kRegScy: background_.scroll_y = val; break;
        case kRegScx: background_.scroll_x = val; break;
        case kRegLy: break;
        case kRegLyc: SetLyc(val); break;
        case kRegBgp: bgp_ = val; break;
        case kRegObp0: obp0_ = val; break;
        case kRegObp1: obp1_ = val; break;
        case kRegWy: window_.SetY(val); break;
        case kRegWx: window_.SetX(val); break;
        default: LOG_ERROR("PPU: Unmapped write {:X} <- {:X}", addr, val);
        }
}

void Ppu::Tick(uint8_t tcycles)
{
    if (!lcd_control_.LcdEnabled())
        return;

    cycles_ += tcycles;

    switch (lcd_status_.GetMode())
    {
    case Mode::HBlank:
    {
        if (cycles_ < kCyclesHBlank)
            return;

        cycles_ -= kCyclesHBlank;

        if (ly_ >= 143)
        {
            interrupts_ |= std::to_underlying(Interrupt::VBlank);
            lcd_status_.SetMode(Mode::VBlank, interrupts_);
            should_draw_frame_ = true;
        }
        else
        {
            if (lcd_control_.WindowEnabled())
                window_.IncrementLine(ly_);

            SetLy(ly_ + 1);
            lcd_status_.SetMode(Mode::Oam, interrupts_);
        }
        break;
    }
    case Mode::VBlank:
    {
        if (cycles_ < kCyclesVBlank)
            return;

        cycles_ -= kCyclesVBlank;
        SetLy(ly_ + 1);

        if (ly_ >= 154)
        {
            lcd_status_.SetMode(Mode::Oam, interrupts_);
            SetLy(0);
            window_.ResetLine();
        }

        break;
    }
    case Mode::Oam:
    {
        if (cycles_ < kCyclesOam)
            return;

        cycles_ -= kCyclesOam;
        lcd_status_.SetMode(Mode::Transfer, interrupts_);
        break;
    }
    case Mode::Transfer:
    {
        if (cycles_ < kCyclesTransfer)
            return;

        cycles_ -= kCyclesTransfer;
        RenderScanline();
        lcd_status_.SetMode(Mode::HBlank, interrupts_);
        break;
    }
    }
}

void Ppu::SetLcdc(uint8_t val)
{
    lcd_control_ = LcdControl{val};
    if (!lcd_control_.LcdEnabled())
    {
        cycles_ = 0;
        ly_ = 0;
        window_.ResetLine();
        lcd_status_.SetMode(Mode::HBlank);
    }
}

void Ppu::SetLy(uint8_t val)
{
    ly_ = val;
    CompareLine();
}

void Ppu::SetLyc(uint8_t val)
{
    lyc_ = val;
    CompareLine();
}

void Ppu::CompareLine()
{
    if (lyc_ == ly_)
    {
        lcd_status_.SetCompareFlag(true);

        if (lcd_status_.LycEqLyEnable())
            interrupts_ |= std::to_underlying(Interrupt::Lcd);
    }
    else
    {
        lcd_status_.SetCompareFlag(false);
    }
}

std::tuple<uint16_t, uint8_t, uint8_t> Ppu::GetBgOrWinTileData(uint8_t lx) const
{
    if (lcd_control_.WindowEnabled() && window_.ContainsPixel(lx, ly_))
    {
        const auto base = lcd_control_.GetWindowBase();
        const auto [x, y] = window_.GetTileMapCoords(lx);
        const auto tile_idx_addr = base + ((y / 8) * (256 / 8)) + (x / 8);
        const auto [x_off, y_off] = window_.GetPixelOffsets(lx, ly_);
        return {tile_idx_addr, x_off, y_off};
    }

    const auto base = lcd_control_.GetBackgroundBase();
    const auto [x, y] = background_.GetTileMapCoords(lx, ly_);
    const auto tile_idx_addr = base + ((y / 8) * (256 / 8)) + (x / 8);
    const auto [x_off, y_off] = Background::GetPixelOffsets(x, y);
    return {tile_idx_addr, x_off, y_off};
}

void Ppu::RenderScanline()
{
    if (lcd_control_.BgWinEnabled())
        RenderBgWinScanline();
}

namespace
{
uint8_t GetPixelColorIndex(uint8_t lo_byte, uint8_t hi_byte, uint8_t pixel)
{
    const auto bit0 = (lo_byte >> pixel) & 1;
    const auto bit1 = ((hi_byte >> pixel) & 1) << 1;
    return static_cast<uint8_t>(bit1 | bit0);
}

Color GetPixelColor(uint8_t palette, uint8_t color_idx)
{
    return Color::FromIndex((palette >> (color_idx << 1)) & 3);
}
}  // namespace

void Ppu::RenderBgWinScanline()
{
    const auto base = static_cast<size_t>(ly_) * kLcdWidth;
    for (uint8_t lx = 0; lx < kLcdWidth; ++lx)
    {
        const auto [tile_idx_addr, x_off, y_off] = GetBgOrWinTileData(lx);
        const uint8_t tile_idx = ReadByte(tile_idx_addr);
        const uint16_t tile_addr = lcd_control_.GetTileAddress(tile_idx);
        const uint8_t lo_byte = ReadByte(tile_addr + y_off);
        const uint8_t hi_byte = ReadByte(tile_addr + y_off + 1);
        const uint8_t color_idx = GetPixelColorIndex(lo_byte, hi_byte, x_off);
        const Color pixel = GetPixelColor(bgp_, color_idx);
        lcd_buf_[base + lx] = pixel;
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

        const uint8_t tile_id = vram_[tile_address + (tile_y * kMapSize) + tile_x];
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
