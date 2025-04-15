#include "core/video/ppu.hpp"

#include <cstddef>
#include <ranges>

#include "core/constants.hpp"
#include "core/sm83/interrupts.hpp"
#include "core/util.hpp"

namespace
{
constexpr size_t kTileSize = 8;
constexpr size_t kTilesPerLine = 32;

constexpr int kCyclesOam = 80;
constexpr int kCyclesTransfer = 172;
constexpr int kCyclesVBlank = 456;
constexpr int kCyclesHBlank = 204;
}  // namespace

namespace gb::video
{
uint8_t Ppu::ReadByte(uint16_t addr) const
{
    if (addr >= kVramStart && addr <= kVramEnd) { return vram_[addr - kVramStart]; }

    if (addr >= kOamStart && addr <= kOamEnd)
    {
        if (!CanAccessOam()) { return 0xff; }
        addr -= kOamStart;
        const Sprite& sprite = oam_[addr / 4];
        switch (addr % 4)
        {
        case 0: return sprite.y;
        case 1: return sprite.x;
        case 2: return sprite.tile_index;
        case 3: return static_cast<uint8_t>(sprite.flags);
        default: std::unreachable();
        }
    }

    switch (addr)
    {
    case kRegLcdc: return static_cast<uint8_t>(lcd_control_);
    case kRegStat: return static_cast<uint8_t>(lcd_status_);
    case kRegScy: return scroll_y_;
    case kRegScx: return scroll_x_;
    case kRegLy: return scan_y_;
    // case kRegLy: return 0x90;
    case kRegLyc: return scan_y_compare_;
    case kRegBgp: return bgp_;
    case kRegObp0: return obp0_;
    case kRegObp1: return obp1_;
    case kRegWy: return window_y_;
    case kRegWx: return window_x_;
    default: LOG_ERROR("PPU: Unmapped read {:X}", addr); return {};
    }
}

void Ppu::WriteByte(uint16_t addr, uint8_t val)
{
    if (addr >= kVramStart && addr <= kVramEnd) { vram_[addr - kVramStart] = val; }
    else if (addr >= kOamStart && addr <= kOamEnd)
    {
        if (!CanAccessOam()) { return; }
        addr -= kOamStart;
        Sprite& sprite = oam_[addr / 4];

        switch (addr % 4)
        {
        case 0: sprite.y = val; break;
        case 1: sprite.x = val; break;
        case 2: sprite.tile_index = val; break;
        case 3: sprite.flags = SpriteFlags{val}; break;
        default: std::unreachable();
        }
    }
    else
    {
        switch (addr)
        {
        case kRegLcdc: SetLcdc(val); break;
        case kRegStat:
            lcd_status_ = LcdStatus{val};
            CompareLine();
            break;
        case kRegScy: scroll_y_ = val; break;
        case kRegScx: scroll_x_ = val; break;
        case kRegLy: break;
        case kRegLyc: SetScanYCompare(val); break;
        case kRegBgp: bgp_ = val; break;
        case kRegObp0: obp0_ = val; break;
        case kRegObp1: obp1_ = val; break;
        case kRegWy: window_y_ = val; break;
        case kRegWx: window_x_ = val; break;
        default: LOG_ERROR("PPU: Unmapped write {:X} <- {:X}", addr, val);
        }
    }
}

namespace
{
uint8_t ScrollAdjustment(uint8_t scroll_x)
{
    switch (scroll_x & 7)
    {
    case 1:
    case 2:
    case 3:
    case 4: return 1;
    case 5:
    case 6:
    case 7: return 2;
    default: return 0;
    }
}
}  // namespace

void Ppu::Tick(uint8_t tcycles)
{
    if (!lcd_control_.LcdEnabled()) { return; }
    cycles_ += tcycles;

    const uint8_t scroll_adjust = ScrollAdjustment(scroll_x_);

    switch (lcd_status_.GetMode())
    {
    case Mode::HBlank:
    {
        if (cycles_ < kCyclesHBlank - scroll_adjust) { return; }
        cycles_ -= kCyclesHBlank - scroll_adjust;

        if (scan_y_ >= 143) [[unlikely]]
        {
            interrupts_ |= sm83::IntVBlank;
            lcd_status_.SetMode(Mode::VBlank, interrupts_);
            should_draw_frame_ = true;
        }
        else
        {
            if (lcd_control_.WindowEnabled() && (window_x_ - 7 < kLcdWidth) &&
                (window_y_ < kLcdHeight) && (scan_y_ >= window_y_))
            {
                ++window_line_counter_;
            }

            SetScanY(scan_y_ + 1);
            lcd_status_.SetMode(Mode::Oam, interrupts_);
        }
        break;
    }
    case Mode::VBlank:
    {
        if (cycles_ < kCyclesVBlank) { return; }
        cycles_ -= kCyclesVBlank;
        SetScanY(scan_y_ + 1);

        if (scan_y_ > 153) [[unlikely]]
        {
            lcd_status_.SetMode(Mode::Oam, interrupts_);
            SetScanY(0);
            window_line_counter_ = 0;
        }
        break;
    }
    case Mode::Oam:
    {
        if (cycles_ < kCyclesOam) { return; }
        cycles_ -= kCyclesOam;

        scanline_sprite_buffer_.clear();

        // Can't use std::views::enumerate here because looks like emscripten doesn't support it yet
        for (size_t oam_idx = 0; oam_idx < oam_.size(); ++oam_idx)
        {
            const auto& sprite = oam_[oam_idx];
            constexpr int kSpriteYOffset = 16;
            if ((sprite.x > 0) && (scan_y_ + kSpriteYOffset >= sprite.y) &&
                (scan_y_ + kSpriteYOffset < sprite.y + lcd_control_.GetSpriteHeight()))
            {
                scanline_sprite_buffer_.emplace_back(oam_idx, sprite);
            }
        }

        constexpr int kMaxSpritesPerScanline = 10;
        if (scanline_sprite_buffer_.size() > kMaxSpritesPerScanline)
        {
            scanline_sprite_buffer_.resize(kMaxSpritesPerScanline);
        }

        std::ranges::sort(
            scanline_sprite_buffer_, [](const auto& a, const auto& b)
            { return std::tie(a.second.x, a.first) > std::tie(b.second.x, b.first); });

        lcd_status_.SetMode(Mode::Transfer, interrupts_);
        break;
    }
    case Mode::Transfer:
    {
        if (cycles_ < kCyclesTransfer + scroll_adjust) { return; }
        cycles_ -= kCyclesTransfer + scroll_adjust;
        RenderScanline();
        lcd_status_.SetMode(Mode::HBlank, interrupts_);
        break;
    }
    }
}

void Ppu::SetLcdc(uint8_t lcdc)
{
    const bool was_enabled = lcd_control_.LcdEnabled();
    lcd_control_ = LcdControl{lcdc};
    if (!lcd_control_.LcdEnabled() && was_enabled) [[unlikely]]
    {
        cycles_ = 0;
        scan_y_ = 0;
        window_line_counter_ = 0;
        lcd_status_.SetMode(Mode::HBlank);
    }
}

void Ppu::SetScanY(uint8_t scan_y)
{
    scan_y_ = scan_y;
    CompareLine();
}

void Ppu::SetScanYCompare(uint8_t scan_y_compare)
{
    scan_y_compare_ = scan_y_compare;
    CompareLine();
}

void Ppu::CompareLine()
{
    if (scan_y_compare_ == scan_y_) [[unlikely]]
    {
        lcd_status_.SetCompareFlag();
        if (lcd_status_.LycEqLyEnable()) { interrupts_ |= sm83::IntLcd; }
    }
    else { lcd_status_.SetCompareFlag(false); }
}

namespace
{
constexpr Color kDmgPalette[4] = {
    {0xff, 0xff, 0xff}, {0xaa, 0xaa, 0xaa}, {0x55, 0x55, 0x55}, {0x00, 0x00, 0x00}};

constexpr uint8_t GetPixelColorIndex(uint8_t lo_byte, uint8_t hi_byte, uint8_t bit_pos)
{
    const auto bit0 = (lo_byte >> bit_pos) & 1;
    const auto bit1 = ((hi_byte >> bit_pos) & 1) << 1;
    return static_cast<uint8_t>(bit1 | bit0);
}

constexpr Color GetPixelColor(uint8_t palette, uint8_t color_idx)
{
    return kDmgPalette[(palette >> (color_idx << 1)) & 3];
}
}  // namespace

void Ppu::RenderScanline()
{
    const auto scanline_start = static_cast<size_t>(scan_y_) * kLcdWidth;
    Color* pixels = &lcd_buf_[scanline_start];

    for (uint8_t scan_x = 0; scan_x < kLcdWidth; ++scan_x)
    {
        Color bg_color = FetchBackgroundPixel(scan_x, scan_y_);
        pixels[scan_x] = bg_color;
        bg_transparency_[scanline_start + scan_x] = (bg_color == kDmgPalette[0]);

        const Color win_color = FetchWindowPixel(scan_x, scan_y_);
        if (!win_color.IsTransparent())
        {
            pixels[scan_x] = win_color;
            bg_transparency_[scanline_start + scan_x] = (win_color == kDmgPalette[0]);
        }
    }
    if (lcd_control_.ObjEnabled()) { RenderSprites(scanline_start); }
}

void Ppu::RenderSprites(size_t scanline_start)
{
    for (const auto& [_, sprite] : scanline_sprite_buffer_)
    {
        const uint8_t tile_index =
            lcd_control_.ObjTallSize() ? sprite.tile_index & ~1 : sprite.tile_index;

        uint8_t row = scan_y_ - (sprite.y - 16);
        if (sprite.flags.YFlip()) { row = lcd_control_.GetSpriteHeight() - 1 - row; }

        const uint16_t tile_addr = kVramStart + ((static_cast<uint16_t>(tile_index * 8) + row) * 2);
        const uint8_t byte1 = ReadByte(tile_addr);
        const uint8_t byte2 = ReadByte(tile_addr + 1);

        for (uint8_t px = 0; px < kTileSize; ++px)
        {
            const uint8_t x_off = (sprite.x - 8) + px;
            if (x_off >= kLcdWidth) { continue; }

            const bool bg_transparent = bg_transparency_[scanline_start + x_off];
            if (sprite.flags.BgWinPriority() && !bg_transparent) { continue; }

            const uint8_t flipped_px = sprite.flags.XFlip() ? px : 7 - px;

            const uint8_t color_idx = GetPixelColorIndex(byte1, byte2, flipped_px);
            if (color_idx == 0) { continue; }

            const uint8_t palette = sprite.flags.DmgPalette() ? obp1_ : obp0_;
            const Color color = GetPixelColor(palette, color_idx);
            lcd_buf_[scanline_start + x_off] = color;
        }
    }
}

Color Ppu::FetchTilePixel(uint8_t tile_idx, uint8_t x_off, uint8_t y_off, uint8_t pal) const
{
    const uint16_t tile_addr = lcd_control_.GetTileAddress(tile_idx);
    const uint8_t byte1 = ReadByte(tile_addr + y_off);
    const uint8_t byte2 = ReadByte(tile_addr + y_off + 1);
    return GetPixelColor(pal, GetPixelColorIndex(byte1, byte2, x_off));
}

Color Ppu::FetchBackgroundPixel(uint8_t scan_x, uint8_t scan_y) const
{
    if (!lcd_control_.BgWinEnabled()) { return kDmgPalette[0]; }

    const uint16_t bg_map_base = lcd_control_.GetBackgroundTileMapAddress();
    const uint8_t bg_map_x = scan_x + scroll_x_;
    const uint8_t bg_map_y = scan_y + scroll_y_;

    const uint16_t tile_idx_addr =
        bg_map_base + ((bg_map_y / kTileSize) * kTilesPerLine) + (bg_map_x / kTileSize);
    const uint8_t tile_idx = ReadByte(tile_idx_addr);

    const uint8_t x_off = 7 - (bg_map_x % kTileSize);
    const uint8_t y_off = 2 * (bg_map_y % kTileSize);
    return FetchTilePixel(tile_idx, x_off, y_off, bgp_);
}

Color Ppu::FetchWindowPixel(uint8_t scan_x, uint8_t scan_y) const
{
    if (scan_y_ < window_y_ || scan_x < window_x_ - 7 || !lcd_control_.WindowEnabled() ||
        !lcd_control_.BgWinEnabled())
    {
        return Color::Transparent();
    }

    const uint16_t win_map_base = lcd_control_.GetWindowTileMapAddress();
    const uint8_t win_map_x = scan_x - (window_x_ - 7);
    const uint8_t win_map_y = window_line_counter_;

    const uint16_t tile_idx_addr =
        win_map_base + ((win_map_y / kTileSize) * kTilesPerLine) + (win_map_x / kTileSize);
    const uint8_t tile_idx = ReadByte(tile_idx_addr);

    const uint8_t x_off = 7 - (win_map_x % kTileSize);
    const uint8_t y_off = 2 * ((scan_y - window_y_) % kTileSize);
    return FetchTilePixel(tile_idx, x_off, y_off, bgp_);
}
}  // namespace gb::video
