#pragma once

#include <bitset>
#include <cstdint>

namespace gb::video
{
class LcdControl
{
public:
    LcdControl() = default;
    explicit LcdControl(uint8_t byte) : bits_(byte) {}

    explicit operator uint8_t() const { return static_cast<uint8_t>(bits_.to_ulong()); }

    [[nodiscard]] constexpr bool BgWinEnabled() const { return bits_.test(0); }
    [[nodiscard]] constexpr bool ObjEnabled() const { return bits_.test(1); }
    [[nodiscard]] constexpr bool ObjTallSize() const { return bits_.test(2); }
    [[nodiscard]] constexpr bool BgTileMap() const { return bits_.test(3); }
    [[nodiscard]] constexpr bool BgWinTileData() const { return bits_.test(4); }
    [[nodiscard]] constexpr bool WindowEnabled() const { return bits_.test(5); }
    [[nodiscard]] constexpr bool WindowTileMap() const { return bits_.test(6); }
    [[nodiscard]] constexpr bool LcdEnabled() const { return bits_.test(7); }

    [[nodiscard]] constexpr uint16_t GetTileAddress(uint8_t tile_index) const
    {
        if (BgWinTileData())
        {
            return 0x8000 + (static_cast<uint16_t>(tile_index) * 16);
        }

        if (tile_index >= 128)
        {
            return 0x8800 + ((static_cast<uint16_t>(tile_index - 128)) * 16);
        }

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

private:
    std::bitset<8> bits_{0x91};
};
}  // namespace gb::video
