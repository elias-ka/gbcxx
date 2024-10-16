#pragma once

#include "util.hpp"

namespace cb
{
    inline constexpr u16 CARTRIDGE_START = 0x0000;
    inline constexpr u16 CARTRIDGE_END = 0x7FFF;
    inline constexpr u16 VRAM_START = 0x8000;
    inline constexpr u16 VRAM_END = 0x9FFF;
    inline constexpr u16 EXTERNAL_RAM_START = 0xA000;
    inline constexpr u16 EXTERNAL_RAM_END = 0xBFFF;
    inline constexpr u16 WORK_RAM_START = 0xC000;
    inline constexpr u16 WORK_RAM_END = 0xDFFF;
    inline constexpr u16 ECHO_RAM_START = 0xE000;
    inline constexpr u16 ECHO_RAM_END = 0xFDFF;
    inline constexpr u16 OAM_START = 0xFE00;
    inline constexpr u16 OAM_END = 0xFE9F;
    inline constexpr u16 NOT_USABLE_START = 0xFEA0;
    inline constexpr u16 NOT_USABLE_END = 0xFEFF;
    inline constexpr u16 IO_START = 0xFF00;
    inline constexpr u16 IO_END = 0xFF7F;
    inline constexpr u16 HIGH_RAM_START = 0xFF80;
    inline constexpr u16 HIGH_RAM_END = 0xFFFE;

    inline constexpr u16 REG_BOOTROM = 0xFF50;

    inline constexpr usz CYCLES_OAM_SCAN = 80;
    inline constexpr usz CYCLES_TRANSFER = 172;
    inline constexpr usz CYCLES_VBLANK = 456;
    inline constexpr usz CYCLES_HBLANK = 204;

    inline constexpr usz SCREEN_WIDTH = 160;
    inline constexpr usz SCREEN_HEIGHT = 144;

    // IO registers
    inline constexpr u16 REG_JOYP = 0xFF00;
    inline constexpr u16 REG_SB = 0xFF01;
    inline constexpr u16 REG_SC = 0xFF02;
    inline constexpr u16 REG_DIV = 0xFF04;
    inline constexpr u16 REG_TIMA = 0xFF05;
    inline constexpr u16 REG_TMA = 0xFF06;
    inline constexpr u16 REG_TAC = 0xFF07;
    inline constexpr u16 REG_IF = 0xFF0F;
    inline constexpr u16 REG_NR10 = 0xFF10;
    inline constexpr u16 REG_NR11 = 0xFF11;
    inline constexpr u16 REG_NR12 = 0xFF12;
    inline constexpr u16 REG_NR13 = 0xFF13;
    inline constexpr u16 REG_NR14 = 0xFF14;
    inline constexpr u16 REG_NR21 = 0xFF16;
    inline constexpr u16 REG_NR22 = 0xFF17;
    inline constexpr u16 REG_NR23 = 0xFF18;
    inline constexpr u16 REG_NR24 = 0xFF19;
    inline constexpr u16 REG_NR30 = 0xFF1A;
    inline constexpr u16 REG_NR31 = 0xFF1B;
    inline constexpr u16 REG_NR32 = 0xFF1C;
    inline constexpr u16 REG_NR33 = 0xFF1D;
    inline constexpr u16 REG_NR34 = 0xFF1E;
    inline constexpr u16 REG_NR41 = 0xFF20;
    inline constexpr u16 REG_NR42 = 0xFF21;
    inline constexpr u16 REG_NR43 = 0xFF22;
    inline constexpr u16 REG_NR44 = 0xFF23;
    inline constexpr u16 REG_NR50 = 0xFF24;
    inline constexpr u16 REG_NR51 = 0xFF25;
    inline constexpr u16 REG_NR52 = 0xFF26;
    inline constexpr u16 WAVE_PATTERN_START = 0xFF30;
    inline constexpr u16 WAVE_PATTERN_END = 0xFF3F;
    inline constexpr u16 REG_LCDC = 0xFF40;
    inline constexpr u16 REG_STAT = 0xFF41;
    inline constexpr u16 REG_SCY = 0xFF42;
    inline constexpr u16 REG_SCX = 0xFF43;
    inline constexpr u16 REG_LY = 0xFF44;
    inline constexpr u16 REG_LYC = 0xFF45;
    inline constexpr u16 REG_BGP = 0xFF47;
    inline constexpr u16 REG_OBP0 = 0xFF48;
    inline constexpr u16 REG_OBP1 = 0xFF49;
    inline constexpr u16 REG_WY = 0xFF4A;
    inline constexpr u16 REG_WX = 0xFF4B;
    inline constexpr u16 REG_KEY1 = 0xFF4D;
    inline constexpr u16 REG_VBK = 0xFF4F;
    inline constexpr u16 REG_HDMA1 = 0xFF51;
    inline constexpr u16 REG_HDMA2 = 0xFF52;
    inline constexpr u16 REG_HDMA3 = 0xFF53;
    inline constexpr u16 REG_HDMA4 = 0xFF54;
    inline constexpr u16 REG_HDMA5 = 0xFF55;
    inline constexpr u16 REG_RP = 0xFF56;
    inline constexpr u16 REG_BCPS = 0xFF68;
    inline constexpr u16 REG_BCPD = 0xFF69;
    inline constexpr u16 REG_OCPS = 0xFF6A;
    inline constexpr u16 REG_OCPD = 0xFF6B;
    inline constexpr u16 REG_OPRI = 0xFF6C;
    inline constexpr u16 REG_SVBK = 0xFF70;
    inline constexpr u16 REG_PCM12 = 0xFF76;
    inline constexpr u16 REG_PCM34 = 0xFF77;
    inline constexpr u16 REG_IE = 0xFFFF;

} // namespace cb
