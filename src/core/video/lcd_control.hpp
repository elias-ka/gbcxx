#pragma once

#include <bitset>
#include <cstdint>

namespace gb::video
{
struct LcdControl : public std::bitset<8>
{
    using std::bitset<8>::bitset;

    explicit operator uint8_t() const { return static_cast<uint8_t>(to_ulong()); }

    [[nodiscard]] constexpr bool BgWinEnabled() const { return test(0); }
    [[nodiscard]] constexpr bool ObjEnabled() const { return test(1); }
    [[nodiscard]] constexpr bool ObjTallSize() const { return test(2); }
    [[nodiscard]] constexpr bool BgTileMap() const { return test(3); }
    [[nodiscard]] constexpr bool BgWinTileData() const { return test(4); }
    [[nodiscard]] constexpr bool WindowEnabled() const { return test(5); }
    [[nodiscard]] constexpr bool WindowTileMap() const { return test(6); }
    [[nodiscard]] constexpr bool LcdEnabled() const { return test(7); }

    [[nodiscard]] constexpr uint16_t GetTileAddress(uint8_t tile_index) const
    {
        if (BgWinTileData()) { return 0x8000 + (static_cast<uint16_t>(tile_index) * 16); }
        if (tile_index >= 128) { return 0x8800 + ((static_cast<uint16_t>(tile_index - 128)) * 16); }
        return 0x9000 + (static_cast<uint16_t>(tile_index) * 16);
    }

    [[nodiscard]] constexpr uint16_t GetBackgroundTileMapAddress() const
    {
        return !BgTileMap() ? 0x9800 : 0x9c00;
    }
    [[nodiscard]] constexpr uint16_t GetWindowTileMapAddress() const
    {
        return !WindowTileMap() ? 0x9800 : 0x9c00;
    }
    [[nodiscard]] constexpr uint8_t GetSpriteHeight() const { return ObjTallSize() ? 16 : 8; }
};
}  // namespace gb::video
