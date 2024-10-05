#pragma once

#include "util.h"

namespace cb
{
    inline constexpr u16 ram_enable_start = 0x0000;
    inline constexpr u16 ram_enable_end = 0x1FFF;
    inline constexpr u16 secondary_bank_register_start = 0x4000;
    inline constexpr u16 secondary_bank_register_end = 0x5FFF;
    inline constexpr u16 rom_bank_start = 0x2000;
    inline constexpr u16 rom_bank_end = 0x3FFF;
    inline constexpr u16 rom_slot_0_start = 0x0000;
    inline constexpr u16 rom_slot_0_end = 0x3FFF;
    inline constexpr u16 rom_slot_1_start = 0x4000;
    inline constexpr u16 rom_slot_1_end = 0x7FFF;
    inline constexpr u16 banking_mode_start = 0x6000;
    inline constexpr u16 banking_mode_end = 0x7FFF;
    inline constexpr u16 external_ram_start = 0xA000;
    inline constexpr u16 external_ram_end = 0xBFFF;

    inline constexpr u16 reg_bootrom = 0xFF50;

    inline constexpr usz cycles_oam_scan = 80;
    inline constexpr usz cycles_transfer = 172;
    inline constexpr usz cycles_vblank = 456;

    inline constexpr usz screen_width = 160;
    inline constexpr usz screen_height = 144;

    inline constexpr u16 reg_lcdc = 0xFF40;
    inline constexpr u16 reg_stat = 0xFF41;
    inline constexpr u16 reg_scy = 0xFF42;
    inline constexpr u16 reg_scx = 0xFF43;
    inline constexpr u16 reg_ly = 0xFF44;
    inline constexpr u16 reg_lyc = 0xFF45;
    inline constexpr u16 reg_bgp = 0xFF47;
    inline constexpr u16 reg_obp0 = 0xFF48;
    inline constexpr u16 reg_obp1 = 0xFF49;
    inline constexpr u16 reg_wx = 0xFF4B;
    inline constexpr u16 reg_wy = 0xFF4A;
} // namespace cb
