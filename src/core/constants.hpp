#pragma once

#include <cstddef>
#include <cstdint>

namespace gbcxx {
constexpr uint16_t CARTRIDGE_START = 0x0000;
constexpr uint16_t CARTRIDGE_END = 0x7FFF;
constexpr uint16_t VRAM_START = 0x8000;
constexpr uint16_t VRAM_END = 0x9FFF;
constexpr uint16_t EXTERNAL_RAM_START = 0xA000;
constexpr uint16_t EXTERNAL_RAM_END = 0xBFFF;
constexpr uint16_t WORK_RAM_START = 0xC000;
constexpr uint16_t WORK_RAM_END = 0xDFFF;
constexpr uint16_t ECHO_RAM_START = 0xE000;
constexpr uint16_t ECHO_RAM_END = 0xFDFF;
constexpr uint16_t OAM_START = 0xFE00;
constexpr uint16_t OAM_END = 0xFE9F;
constexpr uint16_t NOT_USABLE_START = 0xFEA0;
constexpr uint16_t NOT_USABLE_END = 0xFEFF;
constexpr uint16_t IO_START = 0xFF00;
constexpr uint16_t IO_END = 0xFF7F;
constexpr uint16_t HIGH_RAM_START = 0xFF80;
constexpr uint16_t HIGH_RAM_END = 0xFFFE;

constexpr uint16_t REG_BOOTROM = 0xFF50;

constexpr size_t CYCLES_OAM_SCAN = 80;
constexpr size_t CYCLES_TRANSFER = 172;
constexpr size_t CYCLES_VBLANK = 456;
constexpr size_t CYCLES_HBLANK = 204;

constexpr size_t SCREEN_WIDTH = 160;
constexpr size_t SCREEN_HEIGHT = 144;

// IO registers
constexpr uint16_t REG_JOYP = 0xFF00;
constexpr uint16_t REG_SB = 0xFF01;
constexpr uint16_t REG_SC = 0xFF02;
constexpr uint16_t REG_DIV = 0xFF04;
constexpr uint16_t REG_TIMA = 0xFF05;
constexpr uint16_t REG_TMA = 0xFF06;
constexpr uint16_t REG_TAC = 0xFF07;
constexpr uint16_t REG_IF = 0xFF0F;
constexpr uint16_t REG_NR10 = 0xFF10;
constexpr uint16_t REG_NR11 = 0xFF11;
constexpr uint16_t REG_NR12 = 0xFF12;
constexpr uint16_t REG_NR13 = 0xFF13;
constexpr uint16_t REG_NR14 = 0xFF14;
constexpr uint16_t REG_NR21 = 0xFF16;
constexpr uint16_t REG_NR22 = 0xFF17;
constexpr uint16_t REG_NR23 = 0xFF18;
constexpr uint16_t REG_NR24 = 0xFF19;
constexpr uint16_t REG_NR30 = 0xFF1A;
constexpr uint16_t REG_NR31 = 0xFF1B;
constexpr uint16_t REG_NR32 = 0xFF1C;
constexpr uint16_t REG_NR33 = 0xFF1D;
constexpr uint16_t REG_NR34 = 0xFF1E;
constexpr uint16_t REG_NR41 = 0xFF20;
constexpr uint16_t REG_NR42 = 0xFF21;
constexpr uint16_t REG_NR43 = 0xFF22;
constexpr uint16_t REG_NR44 = 0xFF23;
constexpr uint16_t REG_NR50 = 0xFF24;
constexpr uint16_t REG_NR51 = 0xFF25;
constexpr uint16_t REG_NR52 = 0xFF26;
constexpr uint16_t WAVE_PATTERN_START = 0xFF30;
constexpr uint16_t WAVE_PATTERN_END = 0xFF3F;
constexpr uint16_t REG_LCDC = 0xFF40;
constexpr uint16_t REG_STAT = 0xFF41;
constexpr uint16_t REG_SCY = 0xFF42;
constexpr uint16_t REG_SCX = 0xFF43;
constexpr uint16_t REG_LY = 0xFF44;
constexpr uint16_t REG_LYC = 0xFF45;
constexpr uint16_t REG_BGP = 0xFF47;
constexpr uint16_t REG_OBP0 = 0xFF48;
constexpr uint16_t REG_OBP1 = 0xFF49;
constexpr uint16_t REG_WY = 0xFF4A;
constexpr uint16_t REG_WX = 0xFF4B;
constexpr uint16_t REG_KEY1 = 0xFF4D;
constexpr uint16_t REG_VBK = 0xFF4F;
constexpr uint16_t REG_HDMA1 = 0xFF51;
constexpr uint16_t REG_HDMA2 = 0xFF52;
constexpr uint16_t REG_HDMA3 = 0xFF53;
constexpr uint16_t REG_HDMA4 = 0xFF54;
constexpr uint16_t REG_HDMA5 = 0xFF55;
constexpr uint16_t REG_RP = 0xFF56;
constexpr uint16_t REG_BCPS = 0xFF68;
constexpr uint16_t REG_BCPD = 0xFF69;
constexpr uint16_t REG_OCPS = 0xFF6A;
constexpr uint16_t REG_OCPD = 0xFF6B;
constexpr uint16_t REG_OPRI = 0xFF6C;
constexpr uint16_t REG_SVBK = 0xFF70;
constexpr uint16_t REG_PCM12 = 0xFF76;
constexpr uint16_t REG_PCM34 = 0xFF77;
constexpr uint16_t REG_IE = 0xFFFF;

}  // namespace gbcxx
