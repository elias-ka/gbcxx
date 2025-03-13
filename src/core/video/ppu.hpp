#pragma once

#include <array>
#include <cassert>
#include <span>
#include <vector>

#include "core/constants.hpp"
#include "core/video/lcd_control.hpp"
#include "core/video/lcd_status.hpp"

namespace gb::video
{
struct __attribute__((packed)) Color
{
    uint8_t a{0xff};
    uint8_t b{0xff};
    uint8_t g{0xff};
    uint8_t r{0xff};

    static Color FromIndex(uint8_t index)
    {
        switch (index)
        {
        case 0: return Color{0xff, 0xff, 0xff, 0xff};
        case 1: return Color{0xaa, 0xaa, 0xaa, 0xff};
        case 2: return Color{0x55, 0x55, 0x55, 0xff};
        case 3: return Color{0x00, 0x00, 0x00, 0xff};
        default: return Color{0xff, 0x00, 0x00, 0xff};
        }
    }

    static Color Transparent() { return {0, 0, 0, 0}; }

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : a(a), b(b), g(g), r(r) {}

    constexpr auto operator<=>(const Color&) const = default;

    [[nodiscard]] bool IsTransparent() const { return a == 0; }
};

class SpriteFlags
{
public:
    SpriteFlags() = default;
    explicit SpriteFlags(uint8_t byte) : bits_(byte) {}

    explicit operator uint8_t() const { return static_cast<uint8_t>(bits_.to_ulong()); }

    [[nodiscard]] constexpr bool DmgPalette() const { return bits_.test(4); }
    [[nodiscard]] constexpr bool XFlip() const { return bits_.test(5); }
    [[nodiscard]] constexpr bool YFlip() const { return bits_.test(6); }
    [[nodiscard]] constexpr bool BgWinPriority() const { return bits_.test(7); }

private:
    std::bitset<8> bits_;
};

struct Sprite
{
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    SpriteFlags flags;
    size_t oam_index;
};

class Ppu
{
public:
    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    void Tick(uint8_t tcycles);

    uint8_t ConsumeInterrupts() { return interrupts_.Consume(); }

    [[nodiscard]] const std::array<Color, kLcdSize>& GetLcdBuffer() const { return lcd_buf_; }

    [[nodiscard]] bool ShouldDrawFrame() const { return should_draw_frame_; }
    void SetShouldDrawFrame(bool v) { should_draw_frame_ = v; }

    void DrawBackgroundTileMap(std::span<Color> buf) const
    {
        DrawTileMap(buf, lcd_control_.GetBackgroundTileMapAddress());
    }

    void DrawWindowTileMap(std::span<Color> buf) const
    {
        DrawTileMap(buf, lcd_control_.GetWindowTileMapAddress());
    }

private:
    static const int kCyclesOam = 80;
    static const int kCyclesTransfer = 172;
    static const int kCyclesVBlank = 456;
    static const int kCyclesHBlank = 204;

    void SetLcdc(uint8_t lcdc);
    void SetScanY(uint8_t scan_y);
    void SetScanYCompare(uint8_t scan_y_compare);
    void CompareLine();

    void RenderScanline();
    Color FetchBackgroundPixel(uint8_t scan_x, uint8_t scan_y);
    Color FetchWindowPixel(uint8_t scan_x, uint8_t scan_y);

    void DrawTileMap(std::span<Color> buf, uint16_t tile_address) const;

    std::array<Color, kLcdSize> lcd_buf_;
    std::array<uint8_t, 8192> vram_;
    std::array<Sprite, 40> oam_;
    std::vector<std::pair<size_t, Sprite>> scanline_sprite_buffer_;
    std::bitset<kLcdSize> bg_transparency_;
    sm83::Interrupts interrupts_;
    uint16_t cycles_;
    bool should_draw_frame_{};

    LcdControl lcd_control_;
    LcdStatus lcd_status_;

    uint8_t scroll_x_{0};
    uint8_t scroll_y_{0};
    uint8_t bgp_{0xfc};
    uint8_t scan_y_{0};
    uint8_t scan_y_compare_{0};
    uint8_t obp0_{0};
    uint8_t obp1_{0};
    uint8_t window_x_{0};
    uint8_t window_y_{0};
    uint8_t window_line_counter_{0};
};
}  // namespace gb::video
