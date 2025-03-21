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
    if (addr >= kVramStart && addr <= kVramEnd) return vram_[addr - kVramStart];

    if (addr >= kOamStart && addr <= kOamEnd)
    {
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

void Ppu::Tick(uint8_t tcycles)
{
    if (!lcd_control_.LcdEnabled()) return;

    cycles_ += tcycles;

    const uint8_t scroll_adjust = [&]() -> uint8_t
    {
        switch (scroll_x_ & 7)
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
    }();

    switch (lcd_status_.GetMode())
    {
    case Mode::HBlank:
    {
        if (cycles_ < kCyclesHBlank - scroll_adjust) return;

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
        if (cycles_ < kCyclesVBlank) return;

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
        if (cycles_ < kCyclesOam) return;

        cycles_ -= kCyclesOam;

        scanline_sprite_buffer_.clear();

        for (const auto& [oam_index, sprite] : std::views::enumerate(oam_))
        {
            constexpr int kSpriteYOffset = 16;
            if ((sprite.x > 0) && (scan_y_ + kSpriteYOffset >= sprite.y) &&
                (scan_y_ + kSpriteYOffset < sprite.y + lcd_control_.GetSpriteHeight()))
            {
                scanline_sprite_buffer_.emplace_back(oam_index, sprite);
            }
        }

        constexpr int kMaxSpritesPerScanline = 10;
        if (scanline_sprite_buffer_.size() > kMaxSpritesPerScanline)
            scanline_sprite_buffer_.resize(kMaxSpritesPerScanline);

        std::ranges::sort(
            scanline_sprite_buffer_, [](const auto& a, const auto& b)
            { return std::tie(a.second.x, a.first) > std::tie(b.second.x, b.first); });

        lcd_status_.SetMode(Mode::Transfer, interrupts_);
        break;
    }
    case Mode::Transfer:
    {
        if (cycles_ < kCyclesTransfer + scroll_adjust) return;

        cycles_ -= kCyclesTransfer + scroll_adjust;
        RenderScanline();
        lcd_status_.SetMode(Mode::HBlank, interrupts_);
        break;
    }
    }
}  // namespace gb

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
    else lcd_status_.SetCompareFlag(false);
}

namespace
{
Color ColorFromPaletteIndex(uint8_t index)
{
    switch (index)
    {
    case 0: return Color::FromHex("#e0f8d0");
    case 1: return Color::FromHex("#88c070");
    case 2: return Color::FromHex("#346856");
    case 3: return Color::FromHex("#081820");
    default: DIE("ColorFromIndex: Invalid color index");
    }
}

uint8_t GetPixelColorIndex(uint8_t lo_byte, uint8_t hi_byte, uint8_t bit_pos)
{
    GB_ASSERT(bit_pos < 8);
    const auto bit0 = (lo_byte >> bit_pos) & 1;
    const auto bit1 = ((hi_byte >> bit_pos) & 1) << 1;
    return static_cast<uint8_t>(bit1 | bit0);
}

Color GetPixelColor(uint8_t palette, uint8_t color_idx)
{
    GB_ASSERT(color_idx < 4);
    return ColorFromPaletteIndex((palette >> (color_idx << 1)) & 3);
}
}  // namespace

void Ppu::RenderScanline()
{
    const auto scanline_start = static_cast<size_t>(scan_y_) * kLcdWidth;
    for (uint8_t scan_x = 0; scan_x < kLcdWidth; ++scan_x)
    {
        RenderBackground(scanline_start, scan_x);
        RenderWindow(scanline_start, scan_x);
    }
    if (lcd_control_.ObjEnabled()) { RenderSprites(scanline_start); }
}

void Ppu::RenderBackground(size_t scanline_start, uint8_t scan_x)
{
    const Color background_color = FetchBackgroundPixel(scan_x, scan_y_);
    lcd_buf_[scanline_start + scan_x] = background_color;
    bg_transparency_[(scan_y_ * kLcdWidth) + scan_x] =
        (background_color == ColorFromPaletteIndex(0));
}

void Ppu::RenderWindow(size_t scanline_start, uint8_t scan_x)
{
    const Color window_color = FetchWindowPixel(scan_x, scan_y_);
    if (!window_color.IsTransparent())
    {
        lcd_buf_[scanline_start + scan_x] = window_color;
        bg_transparency_[(scan_y_ * kLcdWidth) + scan_x] =
            (window_color == ColorFromPaletteIndex(0));
    }
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
            if (x_off >= kLcdWidth) continue;

            const bool bg_transparent = bg_transparency_[(scan_y_ * kLcdWidth) + x_off];
            if (sprite.flags.BgWinPriority() && !bg_transparent) continue;

            const uint8_t flipped_px = sprite.flags.XFlip() ? px : 7 - px;

            const uint8_t color_idx = GetPixelColorIndex(byte1, byte2, flipped_px);
            if (color_idx == 0) continue;

            const uint8_t palette = sprite.flags.DmgPalette() ? obp1_ : obp0_;
            const Color color = GetPixelColor(palette, color_idx);
            lcd_buf_[scanline_start + x_off] = color;
        }
    }
}

Color Ppu::FetchBackgroundPixel(uint8_t scan_x, uint8_t scan_y)
{
    if (!lcd_control_.BgWinEnabled()) { return ColorFromPaletteIndex(0); }

    const uint16_t bg_map_base = lcd_control_.GetBackgroundTileMapAddress();
    const uint8_t bg_map_x = scan_x + scroll_x_;
    const uint8_t bg_map_y = scan_y + scroll_y_;

    const uint16_t tile_idx_addr =
        bg_map_base + ((bg_map_y / kTileSize) * kTilesPerLine) + (bg_map_x / kTileSize);
    const uint8_t tile_idx = ReadByte(tile_idx_addr);

    const uint8_t x_off = 7 - (bg_map_x % kTileSize);
    const uint8_t y_off = 2 * (bg_map_y % kTileSize);

    const uint16_t tile_addr = lcd_control_.GetTileAddress(tile_idx);
    const uint8_t byte1 = ReadByte(tile_addr + y_off);
    const uint8_t byte2 = ReadByte(tile_addr + y_off + 1);

    const uint8_t color_idx = GetPixelColorIndex(byte1, byte2, x_off);

    return GetPixelColor(bgp_, color_idx);
}

Color Ppu::FetchWindowPixel(uint8_t scan_x, uint8_t scan_y)
{
    if (!lcd_control_.WindowEnabled() || !lcd_control_.BgWinEnabled() || (scan_y_ < window_y_) ||
        (scan_x < window_x_ - 7))
    {
        return Color::Transparent();
    }

    const uint16_t win_map_base = lcd_control_.GetWindowTileMapAddress();
    const uint8_t win_map_x = scan_x - (window_x_ - 7);
    const uint8_t win_map_y = window_line_counter_;

    const uint16_t tile_idx_addr =
        win_map_base + ((win_map_y / kTileSize) * kTilesPerLine) + (win_map_x / kTileSize);
    const uint8_t tile_idx = ReadByte(tile_idx_addr);

    const uint8_t x_off = (window_x_ - scan_x) % kTileSize;
    const uint8_t y_off = 2 * ((scan_y - window_y_) % kTileSize);

    const uint16_t tile_addr = lcd_control_.GetTileAddress(tile_idx);
    const uint8_t byte1 = ReadByte(tile_addr + y_off);
    const uint8_t byte2 = ReadByte(tile_addr + y_off + 1);

    const uint8_t color_idx = GetPixelColorIndex(byte1, byte2, x_off);

    return GetPixelColor(bgp_, color_idx);
}

void Ppu::DrawTileMap(std::span<Color> buf, uint16_t tile_address) const
{
    constexpr size_t kMapWidth = kTilesPerLine * kTileSize;

    GB_ASSERT(buf.size() == (kMapWidth * kMapWidth));

    for (size_t px = 0; px < (kMapWidth * kMapWidth); ++px)
    {
        const size_t x = px % kMapWidth;
        const size_t y = px / kMapWidth;

        const size_t tile_x = x / kTileSize;
        const size_t tile_y = y / kTileSize;

        const uint8_t tile_id =
            vram_[(tile_address - kVramStart) + (tile_y * kTilesPerLine) + tile_x];
        const auto tile_base = static_cast<size_t>(tile_id) * 16;

        const uint8_t tile_row = y % kTileSize;
        const uint8_t tile_col = x % kTileSize;

        const uint8_t lo_byte = vram_[tile_base + (static_cast<size_t>(tile_row) * 2)];
        const uint8_t hi_byte = vram_[tile_base + (static_cast<size_t>(tile_row) * 2) + 1];

        const uint8_t bit_pos = 7 - tile_col;
        const uint8_t color_index = GetPixelColorIndex(lo_byte, hi_byte, bit_pos);

        buf[px] = ColorFromPaletteIndex(color_index);
    }
}
}  // namespace gb::video
