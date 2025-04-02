#include "core/memory/mbc/mbc2.hpp"

#include "core/util.hpp"

namespace gb::memory
{
Mbc2::Mbc2(std::vector<uint8_t> cartrom) : rom_(std::move(cartrom)) {}

uint8_t Mbc2::ReadRom(uint16_t addr) const
{
    if (addr <= 0x3fff) { return rom_[addr]; }
    if (addr <= 0x7fff) { return rom_[(rom_bank_ * 0x4000) + (addr - 0x4000)]; }
    DIE("MBC2: Unmapped ROM read {:X}", addr);
}

uint8_t Mbc2::ReadRam(uint16_t addr) const
{
    if (!ram_enabled_) { return 0xff; }
    return ram_[addr - 0xa000] & 0xf;
}

void Mbc2::WriteRom(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1fff)
    {
        if (GetBit<8>(addr) != 0) { return; }
        ram_enabled_ = val == 0xa;
    }
    else if (addr <= 0x3fff)
    {
        if (GetBit<8>(addr) != 1) { return; }
        rom_bank_ = ((val == 0) ? 1 : val) & 0xf;
    }
    else if (addr <= 0x7fff) {}
    else { DIE("MBC2: Unmapped ROM write {:X} <- {:X}", addr, val); }
}

void Mbc2::WriteRam(uint16_t addr, uint8_t val)
{
    if (!ram_enabled_) { return; }
    ram_[addr - 0xa000] = val & 0xf;
}

void Mbc2::LoadRam(std::ifstream& save_file)
{
    save_file.read(reinterpret_cast<char*>(ram_.data()), static_cast<std::streamsize>(ram_.size()));
}

void Mbc2::SaveRam(std::ofstream& save_file) const
{
    save_file.write(reinterpret_cast<const char*>(ram_.data()),
                    static_cast<std::streamsize>(ram_.size()));
}

}  // namespace gb::memory
