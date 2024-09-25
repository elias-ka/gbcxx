#pragma once

#include "util.h"

namespace cb
{
    namespace video
    {
        inline constexpr usz cycles_oam_scan = 21;
        inline constexpr usz cycles_transfer = 43;
        inline constexpr usz cycles_hblank = 50;
        inline constexpr usz cycles_vblank = 114;

        inline constexpr usz screen_width = 160;
        inline constexpr usz screen_height = 144;

        namespace reg
        {
            inline constexpr u16 lcdc = 0xFF40;
            inline constexpr u16 stat = 0xFF41;
            inline constexpr u16 scy = 0xFF42;
            inline constexpr u16 scx = 0xFF43;
            inline constexpr u16 ly = 0xFF44;
            inline constexpr u16 lyc = 0xFF45;
            inline constexpr u16 bgp = 0xFF47;
            inline constexpr u16 obp0 = 0xFF48;
            inline constexpr u16 obp1 = 0xFF49;
            inline constexpr u16 wx = 0xFF4B;
            inline constexpr u16 wy = 0xFF4A;
        } // namespace reg
    } // namespace video
} // namespace cb
