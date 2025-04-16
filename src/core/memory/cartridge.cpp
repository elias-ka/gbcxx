#include "core/memory/cartridge.hpp"

#include "core/constants.hpp"
#include "core/memory/mbc.hpp"
#include "core/util.hpp"

namespace gb::memory
{
namespace
{
constexpr std::array<std::string_view, 256> kCartridgeNameTable = []
{
    std::array<std::string_view, 256> table;
    table.fill("Unknown");

    table[0x00] = "ROM ONLY";
    table[0x01] = "MBC1";
    table[0x02] = "MBC1+RAM";
    table[0x03] = "MBC1+RAM+BATTERY";
    table[0x05] = "MBC2";
    table[0x06] = "MBC2+BATTERY";
    table[0x08] = "ROM+RAM";
    table[0x09] = "ROM+RAM+BATTERY";
    table[0x0b] = "MMM01";
    table[0x0c] = "MMM01+RAM";
    table[0x0d] = "MMM01+RAM+BATTERY";
    table[0x0f] = "MBC3+TIMER+BATTERY";
    table[0x10] = "MBC3+TIMER+RAM+BATTERY";
    table[0x11] = "MBC3";
    table[0x12] = "MBC3+RAM";
    table[0x13] = "MBC3+RAM+BATTERY";
    table[0x19] = "MBC5";
    table[0x1a] = "MBC5+RAM";
    table[0x1b] = "MBC5+RAM+BATTERY";
    table[0x1c] = "MBC5+RUMBLE";
    table[0x1d] = "MBC5+RUMBLE+RAM";
    table[0x1e] = "MBC5+RUMBLE+RAM+BATTERY";
    table[0x20] = "MBC6";
    table[0x22] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
    table[0xfc] = "POCKET CAMERA";
    table[0xfd] = "BANDAI TAMA5";
    table[0xfe] = "HuC3";
    table[0xff] = "HuC1+RAM+BATTERY";

    return table;
}();

constexpr std::array<uint8_t, 11> kBatterySupportedCodes = {
    0x03, 0x06, 0x09, 0x0d, 0x0f, 0x10, 0x13, 0x1b, 0x1e, 0x22, 0xff,
};
}  // namespace

Cartridge Cartridge::FromRom(std::vector<uint8_t> rom)
{
    const uint8_t code = rom[0x147];
    const auto cart_name = kCartridgeNameTable[code];
    LOG_INFO("Cartridge: {}", cart_name);

    Cartridge cart;

    cart.mbc = [&]() -> std::unique_ptr<Mbc>
    {
        switch (code)
        {
        case 0x00: return std::make_unique<Mbc0>(std::move(rom));
        case 0x01:
        case 0x02:
        case 0x03: return std::make_unique<Mbc1>(std::move(rom));
        case 0x05:
        case 0x06: return std::make_unique<Mbc2>(std::move(rom));
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13: return std::make_unique<Mbc3>(std::move(rom));
        default: DIE("MBC: Unimplemented cartridge code {:X}", code);
        }
    }();

    cart.has_battery = std::ranges::contains(kBatterySupportedCodes, code);
    return cart;
}

uint8_t Cartridge::ReadByte(uint16_t addr) const
{
    if (addr >= kCartridgeStart && addr <= kCartridgeEnd) { return mbc->ReadRom(addr); }
    if (addr >= kExternalRamStart && addr <= kExternalRamEnd) { return mbc->ReadRam(addr); }
    LOG_ERROR("Cartridge: Unmapped read {:X}", addr);
    return 0;
}

void Cartridge::WriteByte(uint16_t addr, uint8_t val) const
{
    if (addr >= kCartridgeStart && addr <= kCartridgeEnd) { mbc->WriteRom(addr, val); }
    else if (addr >= kExternalRamStart && addr <= kExternalRamEnd) { mbc->WriteRam(addr, val); }
    else { LOG_ERROR("Cartridge: Unmapped write {:X} <- {:X}", addr, val); }
}

void Cartridge::LoadRam(std::ifstream& save_file) const { mbc->LoadRam(save_file); }
void Cartridge::SaveRam(std::ofstream& save_file) const { mbc->SaveRam(save_file); }
}  // namespace gb::memory
