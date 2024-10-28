#pragma once

#include "util.hpp"

namespace gbcxx {
constexpr u16 CARTRIDGE_START = 0x0000;
constexpr u16 CARTRIDGE_END = 0x7FFF;
constexpr u16 VRAM_START = 0x8000;
constexpr u16 VRAM_END = 0x9FFF;
constexpr u16 EXTERNAL_RAM_START = 0xA000;
constexpr u16 EXTERNAL_RAM_END = 0xBFFF;
constexpr u16 WORK_RAM_START = 0xC000;
constexpr u16 WORK_RAM_END = 0xDFFF;
constexpr u16 ECHO_RAM_START = 0xE000;
constexpr u16 ECHO_RAM_END = 0xFDFF;
constexpr u16 OAM_START = 0xFE00;
constexpr u16 OAM_END = 0xFE9F;
constexpr u16 NOT_USABLE_START = 0xFEA0;
constexpr u16 NOT_USABLE_END = 0xFEFF;
constexpr u16 IO_START = 0xFF00;
constexpr u16 IO_END = 0xFF7F;
constexpr u16 HIGH_RAM_START = 0xFF80;
constexpr u16 HIGH_RAM_END = 0xFFFE;

constexpr u16 REG_BOOTROM = 0xFF50;

constexpr usz CYCLES_OAM_SCAN = 80;
constexpr usz CYCLES_TRANSFER = 172;
constexpr usz CYCLES_VBLANK = 456;
constexpr usz CYCLES_HBLANK = 204;

constexpr usz SCREEN_WIDTH = 160;
constexpr usz SCREEN_HEIGHT = 144;

// IO registers
constexpr u16 REG_JOYP = 0xFF00;
constexpr u16 REG_SB = 0xFF01;
constexpr u16 REG_SC = 0xFF02;
constexpr u16 REG_DIV = 0xFF04;
constexpr u16 REG_TIMA = 0xFF05;
constexpr u16 REG_TMA = 0xFF06;
constexpr u16 REG_TAC = 0xFF07;
constexpr u16 REG_IF = 0xFF0F;
constexpr u16 REG_NR10 = 0xFF10;
constexpr u16 REG_NR11 = 0xFF11;
constexpr u16 REG_NR12 = 0xFF12;
constexpr u16 REG_NR13 = 0xFF13;
constexpr u16 REG_NR14 = 0xFF14;
constexpr u16 REG_NR21 = 0xFF16;
constexpr u16 REG_NR22 = 0xFF17;
constexpr u16 REG_NR23 = 0xFF18;
constexpr u16 REG_NR24 = 0xFF19;
constexpr u16 REG_NR30 = 0xFF1A;
constexpr u16 REG_NR31 = 0xFF1B;
constexpr u16 REG_NR32 = 0xFF1C;
constexpr u16 REG_NR33 = 0xFF1D;
constexpr u16 REG_NR34 = 0xFF1E;
constexpr u16 REG_NR41 = 0xFF20;
constexpr u16 REG_NR42 = 0xFF21;
constexpr u16 REG_NR43 = 0xFF22;
constexpr u16 REG_NR44 = 0xFF23;
constexpr u16 REG_NR50 = 0xFF24;
constexpr u16 REG_NR51 = 0xFF25;
constexpr u16 REG_NR52 = 0xFF26;
constexpr u16 WAVE_PATTERN_START = 0xFF30;
constexpr u16 WAVE_PATTERN_END = 0xFF3F;
constexpr u16 REG_LCDC = 0xFF40;
constexpr u16 REG_STAT = 0xFF41;
constexpr u16 REG_SCY = 0xFF42;
constexpr u16 REG_SCX = 0xFF43;
constexpr u16 REG_LY = 0xFF44;
constexpr u16 REG_LYC = 0xFF45;
constexpr u16 REG_BGP = 0xFF47;
constexpr u16 REG_OBP0 = 0xFF48;
constexpr u16 REG_OBP1 = 0xFF49;
constexpr u16 REG_WY = 0xFF4A;
constexpr u16 REG_WX = 0xFF4B;
constexpr u16 REG_KEY1 = 0xFF4D;
constexpr u16 REG_VBK = 0xFF4F;
constexpr u16 REG_HDMA1 = 0xFF51;
constexpr u16 REG_HDMA2 = 0xFF52;
constexpr u16 REG_HDMA3 = 0xFF53;
constexpr u16 REG_HDMA4 = 0xFF54;
constexpr u16 REG_HDMA5 = 0xFF55;
constexpr u16 REG_RP = 0xFF56;
constexpr u16 REG_BCPS = 0xFF68;
constexpr u16 REG_BCPD = 0xFF69;
constexpr u16 REG_OCPS = 0xFF6A;
constexpr u16 REG_OCPD = 0xFF6B;
constexpr u16 REG_OPRI = 0xFF6C;
constexpr u16 REG_SVBK = 0xFF70;
constexpr u16 REG_PCM12 = 0xFF76;
constexpr u16 REG_PCM34 = 0xFF77;
constexpr u16 REG_IE = 0xFFFF;

}  // namespace gbcxx
