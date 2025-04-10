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

    static constexpr Color Transparent() { return {0, 0, 0, 0}; }

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : a(a), b(b), g(g), r(r) {}

    constexpr auto operator<=>(const Color&) const = default;

    [[nodiscard]] constexpr bool IsTransparent() const { return a == 0; }
};

struct SpriteFlags : std::bitset<8>
{
public:
    using std::bitset<8>::bitset;

    explicit operator uint8_t() const { return static_cast<uint8_t>(to_ulong()); }

    [[nodiscard]] constexpr bool DmgPalette() const { return test(4); }
    [[nodiscard]] constexpr bool XFlip() const { return test(5); }
    [[nodiscard]] constexpr bool YFlip() const { return test(6); }
    [[nodiscard]] constexpr bool BgWinPriority() const { return test(7); }
};

struct Sprite
{
    uint8_t y{};
    uint8_t x{};
    uint8_t tile_index{};
    SpriteFlags flags;
};

using LcdBuffer = std::array<Color, kLcdSize>;

class Ppu
{
public:
    void Tick(uint8_t tcycles);

    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    [[nodiscard]] uint8_t ConsumeInterrupts() { return std::exchange(interrupts_, 0); }

    [[nodiscard]] const LcdBuffer& GetLcdBuffer() const { return lcd_buf_; }

    [[nodiscard]] bool ShouldDrawFrame() const { return should_draw_frame_; }
    void SetShouldDrawFrame(bool should_draw_frame) { should_draw_frame_ = should_draw_frame; }

private:
    void SetLcdc(uint8_t lcdc);
    void SetScanY(uint8_t scan_y);
    void SetScanYCompare(uint8_t scan_y_compare);
    void CompareLine();

    void RenderScanline();
    void RenderSprites(size_t scanline_start);

    [[nodiscard]] Color FetchTilePixel(uint8_t tile_idx, uint8_t x_off, uint8_t y_off,
                                       uint8_t pal) const;
    [[nodiscard]] Color FetchBackgroundPixel(uint8_t scan_x, uint8_t scan_y) const;
    [[nodiscard]] Color FetchWindowPixel(uint8_t scan_x, uint8_t scan_y) const;

    LcdBuffer lcd_buf_{};
    std::array<uint8_t, 8192> vram_{};
    std::array<Sprite, 40> oam_{};
    std::vector<std::pair<size_t, Sprite>> scanline_sprite_buffer_;
    std::bitset<kLcdSize> bg_transparency_;
    uint8_t interrupts_;
    uint16_t cycles_{};
    bool should_draw_frame_{};

    LcdControl lcd_control_{0x91};
    LcdStatus lcd_status_{0x85};

    uint8_t scroll_x_{};
    uint8_t scroll_y_{};
    uint8_t bgp_{0xfc};
    uint8_t scan_y_{};
    uint8_t scan_y_compare_{};
    uint8_t obp0_{};
    uint8_t obp1_{};
    uint8_t window_x_{};
    uint8_t window_y_{};
    uint8_t window_line_counter_{};
};
}  // namespace gb::video
