#include "core/memory/mbc/mbc3.hpp"

#include "core/util.hpp"

namespace gb::memory
{
Mbc3::Mbc3(std::vector<uint8_t> cartrom) : rom_(std::move(cartrom)), ram_(32_KiB) {}

uint8_t Mbc3::ReadRom(uint16_t addr) const
{
    if (addr <= 0x3fff) { return rom_[addr]; }
    if (addr <= 0x7fff) { return rom_[(rom_bank_ * 0x4000) | (addr % 0x4000)]; }
    DIE("MBC3: Unmapped ROM read {:X}", addr);
}

uint8_t Mbc3::ReadRam(uint16_t addr) const
{
    if (!ram_enabled_) { return 0xff; }
    if (addr >= 0xa000 && addr <= 0xbfff) { return ram_[(addr - 0xa000) + (ram_bank_ * 0x2000)]; }
    DIE("MBC3: Unmapped RAM read {:X}", addr);
}

void Mbc3::WriteRom(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1fff) { ram_enabled_ = (val & 0xf) == 0xa; }
    else if (addr <= 0x3fff) { rom_bank_ = std::max<size_t>(1, val & 0x7f); }
    else if (addr <= 0x5fff) { ram_bank_ = val & 0xf; }
    else if (addr >= 0x6000 && addr <= 0x7fff)
    {
        // 6000-7FFF - Latch Clock Data, unsure if this is even needed
        return;
    }
    else { DIE("MBC3: Unmapped register write {:X} <- {:X}", addr, val); }
}

void Mbc3::WriteRam(uint16_t addr, uint8_t val)
{
    if (!ram_enabled_) { return; }
    if (addr >= 0xa000 && addr <= 0xbfff) { ram_[(addr - 0xa000) + (ram_bank_ * 0x2000)] = val; }
    else { DIE("MBC3: Unmapped RAM write {:X} <- {:X}", addr, val); }
}

void Mbc3::LoadRam(std::ifstream& save_file)
{
    save_file.read(reinterpret_cast<char*>(ram_.data()), static_cast<std::streamsize>(ram_.size()));
}

void Mbc3::SaveRam(std::ofstream& save_file) const
{
    save_file.write(reinterpret_cast<const char*>(ram_.data()),
                    static_cast<std::streamsize>(ram_.size()));
}

}  // namespace gb::memory
