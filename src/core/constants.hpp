#pragma once

#include <cstddef>
#include <cstdint>

namespace gb
{
constexpr int kLcdWidth = 160;
constexpr int kLcdHeight = 144;
constexpr auto kLcdSize = static_cast<size_t>(kLcdWidth) * kLcdHeight;

constexpr uint16_t kCartridgeStart = 0x0000;
constexpr uint16_t kCartridgeEnd = 0x7fff;
constexpr uint16_t kVramStart = 0x8000;
constexpr uint16_t kVramEnd = 0x9fff;
constexpr uint16_t kExternalRamStart = 0xa000;
constexpr uint16_t kExternalRamEnd = 0xbfff;
constexpr uint16_t kWorkRamStart = 0xc000;
constexpr uint16_t kWorkRamEnd = 0xdfff;
constexpr uint16_t kEchoRamStart = 0xe000;
constexpr uint16_t kEchoRamEnd = 0xfdff;
constexpr uint16_t kOamStart = 0xfe00;
constexpr uint16_t kOamEnd = 0xfe9f;
constexpr uint16_t kNotUsableStart = 0xfea0;
constexpr uint16_t kNotUsableEnd = 0xfeff;
constexpr uint16_t kIoStart = 0xff00;
constexpr uint16_t kIoEnd = 0xff7f;
constexpr uint16_t kHighRamStart = 0xff80;
constexpr uint16_t kHighRamEnd = 0xfffe;

constexpr uint16_t kRegJoyp = 0xff00;
constexpr uint16_t kRegSb = 0xff01;
constexpr uint16_t kRegSc = 0xff02;
constexpr uint16_t kRegDiv = 0xff04;
constexpr uint16_t kRegTima = 0xff05;
constexpr uint16_t kRegTma = 0xff06;
constexpr uint16_t kRegTac = 0xff07;
constexpr uint16_t kRegIf = 0xff0f;
constexpr uint16_t kRegNr10 = 0xff10;
constexpr uint16_t kRegNr11 = 0xff11;
constexpr uint16_t kRegNr12 = 0xff12;
constexpr uint16_t kRegNr13 = 0xff13;
constexpr uint16_t kRegNr14 = 0xff14;
constexpr uint16_t kRegNr21 = 0xff16;
constexpr uint16_t kRegNr22 = 0xff17;
constexpr uint16_t kRegNr23 = 0xff18;
constexpr uint16_t kRegNr24 = 0xff19;
constexpr uint16_t kRegNr30 = 0xff1a;
constexpr uint16_t kRegNr31 = 0xff1b;
constexpr uint16_t kRegNr32 = 0xff1c;
constexpr uint16_t kRegNr33 = 0xff1d;
constexpr uint16_t kRegNr34 = 0xff1e;
constexpr uint16_t kRegNr41 = 0xff20;
constexpr uint16_t kRegNr42 = 0xff21;
constexpr uint16_t kRegNr43 = 0xff22;
constexpr uint16_t kRegNr44 = 0xff23;
constexpr uint16_t kRegNr50 = 0xff24;
constexpr uint16_t kRegNr51 = 0xff25;
constexpr uint16_t kRegNr52 = 0xff26;
constexpr uint16_t kWavePatternStart = 0xff30;
constexpr uint16_t kWavePatternEnd = 0xff3f;
constexpr uint16_t kRegLcdc = 0xff40;
constexpr uint16_t kRegStat = 0xff41;
constexpr uint16_t kRegScy = 0xff42;
constexpr uint16_t kRegScx = 0xff43;
constexpr uint16_t kRegLy = 0xff44;
constexpr uint16_t kRegLyc = 0xff45;
constexpr uint16_t kRegOamDma = 0xff46;
constexpr uint16_t kRegBgp = 0xff47;
constexpr uint16_t kRegObp0 = 0xff48;
constexpr uint16_t kRegObp1 = 0xff49;
constexpr uint16_t kRegWy = 0xff4a;
constexpr uint16_t kRegWx = 0xff4b;
constexpr uint16_t kRegKey1 = 0xff4d;
constexpr uint16_t kRegVbk = 0xff4f;
constexpr uint16_t kRegBootrom = 0xff50;
constexpr uint16_t kRegHdma1 = 0xff51;
constexpr uint16_t kRegHdma2 = 0xff52;
constexpr uint16_t kRegHdma3 = 0xff53;
constexpr uint16_t kRegHdma4 = 0xff54;
constexpr uint16_t kRegHdma5 = 0xff55;
constexpr uint16_t kRegRp = 0xff56;
constexpr uint16_t kRegBcps = 0xff68;
constexpr uint16_t kRegBcpd = 0xff69;
constexpr uint16_t kRegOcps = 0xff6a;
constexpr uint16_t kRegOcpd = 0xff6b;
constexpr uint16_t kRegOpri = 0xff6c;
constexpr uint16_t kRegSvbk = 0xff70;
constexpr uint16_t kRegPcm12 = 0xff76;
constexpr uint16_t kRegPcm34 = 0xff77;
constexpr uint16_t kRegIe = 0xffff;
}  // namespace gb
