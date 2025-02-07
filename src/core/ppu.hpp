#pragma once

#include <array>
#include <cassert>

#include "core/constants.hpp"
#include "util.hpp"

namespace gb
{
class Core;

struct __attribute__((packed)) Color
{
    uint8_t r{};
    uint8_t g{};
    uint8_t b{};
    uint8_t a{0xff};

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : r(r), g(g), b(b), a(a) {}
};

class Ppu
{
public:
    using Scanline = std::array<Color, kLcdWidth>;
    using ScanlineDrawCallback = std::function<void(const Scanline&, uint8_t line)>;

    explicit Ppu(Core& gb) : gb_(gb) {}

    [[nodiscard]] uint8_t Read(uint16_t addr) const;
    void Write(uint16_t addr, uint8_t val);

    [[nodiscard]] std::span<const uint8_t> GetVram() const { return vram_; }

    [[nodiscard]] uint16_t GetTileMapBase() const
    {
        return (lcdc_ & ControlBit::BgTileMap) ? 0x1c00 : 0x1800;
    }

    [[nodiscard]] uint16_t GetTileDataBase() const
    {
        return (lcdc_ & ControlBit::BgAndWinTileData) ? 0x0000 : 0x0800;
    }

    void Tick();

    void SetScanlineDrawCallback(ScanlineDrawCallback cb)
    {
        scanline_draw_callback_ = std::move(cb);
    }

    [[nodiscard]] bool IsFrameReady() const { return frame_ready_; }
    void SetFrameReady(bool ready) { frame_ready_ = ready; }

    void DrawBackgroundtileMap(std::vector<gb::Color>& buf) const
    {
        DrawTileMap(buf, GetTileMapBase());
    }

private:
    static const int kDotsOamScan = 80;
    static const int kDotsTransfer = 172;
    static const int kDotsVBlank = 456;
    static const int kDotsHBlank = 204;

    enum class Mode : uint8_t
    {
        HBlank = 0,
        VBlank = 1,
        OamScan = 2,
        Transfer = 3
    };

    static constexpr std::string_view ModeToString(Mode mode)
    {
        switch (mode)
        {
            case Mode::HBlank: return "HBlank";
            case Mode::VBlank: return "VBlank";
            case Mode::OamScan: return "OAM Scan";
            case Mode::Transfer: return "Transfer";
        }
    }

    enum ControlBit : uint8_t
    {
        BgAndWinDisplay = 1 << 0,
        ObjDisplay = 1 << 1,
        ObjSize = 1 << 2,
        BgTileMap = 1 << 3,
        BgAndWinTileData = 1 << 4,
        WindowDisplay = 1 << 5,
        WindowTileMap = 1 << 6,
        LcdDisplay = 1 << 7,
    };

    enum StatusBit : uint8_t
    {
        LycEqLyInterrupt = 1 << 2,
        Mode0Condition = 1 << 3,
        Mode1Condition = 1 << 4,
        Mode2Condition = 1 << 5,
        LycEqLyEnable = 1 << 6,
    };

    void SwitchMode(Mode new_mode);
    void IncrementLy();

    void OnHBlank();
    void OnVBlank();
    void OnOamScan();
    void OnTransfer();

    void RenderScanline();
    void RenderBackgroundScanline(Scanline& scanline) const;

    void DrawTileMap(std::vector<gb::Color>& buf, uint16_t tile_address) const;

    std::array<uint8_t, 8_KiB> vram_;
    std::array<uint8_t, 0xa0> oam_;
    bool frame_ready_{false};
    Mode mode_{Mode::OamScan};
    Core& gb_;
    uint16_t dots_{};
    std::array<Color, 4> bg_palette_{{{0xff, 0xff, 0xff, 0xff},
                                      {0xcc, 0xcc, 0xcc, 0xff},
                                      {0x77, 0x77, 0x77, 0xff},
                                      {0x00, 0x00, 0x00, 0xff}}};
    ScanlineDrawCallback scanline_draw_callback_;

    uint8_t lcdc_{0x91};
    uint8_t stat_{0x85};
    uint8_t bgp_{0xfc};
    uint8_t lx_{0x00};
    uint8_t ly_{0x00};
    uint8_t lyc_{0x00};
    uint8_t scx_{0x00};
    uint8_t scy_{0x00};
    uint8_t obp0_{0x00};
    uint8_t obp1_{0x00};
    uint8_t wy_{0x00};
    uint8_t wx_{0x00};
};
}  // namespace gb
