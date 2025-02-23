#pragma once

#include <array>
#include <cassert>
#include <span>

#include "core/constants.hpp"
#include "core/video/lcd_control.hpp"
#include "core/video/lcd_status.hpp"
#include "core/video/window.hpp"

namespace gb
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

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : a(a), b(b), g(g), r(r) {}
};

class Ppu
{
public:
    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val);

    void Tick(uint8_t tcycles);

    uint8_t GetAndClearInterrupts() { return std::exchange(interrupts_, 0); }

    [[nodiscard]] const std::array<Color, kLcdSize>& GetLcdBuffer() const { return lcd_buf_; }

    [[nodiscard]] bool ShouldDrawFrame() const { return should_draw_frame_; }
    void SetShouldDrawFrame(bool v) { should_draw_frame_ = v; }

private:
    static const int kCyclesOam = 80;
    static const int kCyclesTransfer = 172;
    static const int kCyclesVBlank = 456;
    static const int kCyclesHBlank = 204;

    struct Background
    {
        uint8_t scroll_x{};
        uint8_t scroll_y{};

        [[nodiscard]] std::tuple<uint8_t, uint8_t> GetTileMapCoords(uint8_t lx, uint8_t ly) const
        {
            return {lx + scroll_x, ly + scroll_y};
        }

        [[nodiscard]] static std::tuple<uint8_t, uint8_t> GetPixelOffsets(uint8_t x, uint8_t y)
        {
            const uint8_t x_off = 7 - (x % 8);
            const uint8_t y_off = 2 * (y % 8);
            return {x_off, y_off};
        }
    };

    void SetLcdc(uint8_t val);
    void SetLy(uint8_t val);
    void SetLyc(uint8_t val);
    void CompareLine();

    [[nodiscard]] std::tuple<uint16_t, uint8_t, uint8_t> GetBgOrWinTileData(uint8_t lx) const;

    void RenderScanline();
    void RenderBgWinScanline();

    void DrawTileMap(std::span<gb::Color> buf, uint16_t tile_address) const;

    std::array<uint8_t, 8192> vram_;
    std::array<uint8_t, 160> oam_;
    std::array<Color, kLcdSize> lcd_buf_;
    uint16_t cycles_;
    uint8_t interrupts_;
    bool should_draw_frame_{};

    LcdControl lcd_control_;
    LcdStatus lcd_status_;
    Background background_;
    Window window_;

    uint8_t bgp_{0xfc};
    uint8_t ly_{0x00};
    uint8_t lyc_{0x00};
    uint8_t obp0_{0x00};
    uint8_t obp1_{0x00};
};
}  // namespace gb
