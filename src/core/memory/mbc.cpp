#include "core/memory/mbc.hpp"

#include "core/util.hpp"

namespace
{
size_t CountRamBanks(uint8_t val)
{
    switch (val)
    {
    case 1:
    case 2: return 1;
    case 3: return 4;
    case 4: return 16;
    case 5: return 8;
    default: return 0;
    }
}

size_t CountRomBanks(uint8_t val)
{
    if (val <= 8) { return static_cast<size_t>(2) << val; }
    return 0;
}
}  // namespace

namespace gb::memory
{
// MBC0
Mbc0::Mbc0(std::vector<uint8_t> cartrom) : rom_(std::move(cartrom)) {}
uint8_t Mbc0::ReadRom(uint16_t addr) const { return rom_[addr]; }
uint8_t Mbc0::ReadRam(uint16_t /*address*/) const { return 0; }
void Mbc0::WriteRom(uint16_t /*address*/, uint8_t /*value*/) {}
void Mbc0::WriteRam(uint16_t /*address*/, uint8_t /*value*/) {}
void Mbc0::LoadRam(std::ifstream& /*save_file*/) {}
void Mbc0::SaveRam(std::ofstream& /*save_file*/) const {}

Mbc1::Mbc1(std::vector<uint8_t> cartrom)
    : rom_(std::move(cartrom)),
      nr_rom_banks_(CountRomBanks(rom_[0x148])),
      nr_ram_banks_(CountRamBanks(rom_[0x149])),
      ram_(nr_rom_banks_ * 0x2000)
{
}

// MBC1
uint8_t Mbc1::ReadRom(uint16_t addr) const
{
    const auto bank = [&] -> size_t
    {
        if (addr < 0x4000)
        {
            if (banking_mode_ == 0) { return 0; }
            return rom_bank_ & 0xe0;
        }
        return rom_bank_;
    }();

    return rom_[(bank * 0x4000) + (addr % 0x4000)];
}

uint8_t Mbc1::ReadRam(uint16_t addr) const
{
    if (!ram_enabled_) { return 0xff; }
    const size_t ram_bank = banking_mode_ == 1 ? ram_bank_ : 0;
    return ram_[(ram_bank * 0x2000) + (addr % 0x2000)];
}

void Mbc1::WriteRom(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1fff) { ram_enabled_ = ((val & 0xf) == 0xa); }
    else if (addr <= 0x3fff)
    {
        const uint8_t lower_bits = (val & 0x1f) == 0 ? 1 : (val & 0x1f);
        rom_bank_ = ((rom_bank_ & 0x60) | lower_bits) % nr_rom_banks_;
    }
    else if (addr <= 0x5fff)
    {
        if (nr_rom_banks_ > 32)
        {
            const uint8_t upper_bits = (val & 3) % (nr_rom_banks_ >> 5);
            rom_bank_ = ((rom_bank_ & 0x1f) | static_cast<size_t>(upper_bits << 5));
        }

        if (nr_ram_banks_ > 1) { ram_bank_ = val & 3; }
    }
    else if (addr <= 0x7fff) { banking_mode_ = val & 1; }
    else { DIE("MBC1: Unmapped ROM write {:X} <- {:X}", addr, val); }
}

void Mbc1::WriteRam(uint16_t addr, uint8_t val)
{
    if (!ram_enabled_) { return; }
    const size_t ram_bank = (banking_mode_ == 1) ? ram_bank_ : 0;
    ram_[(ram_bank * 0x2000) + (addr % 0x2000)] = val;
}

void Mbc1::LoadRam(std::ifstream& save_file)
{
    save_file.read(reinterpret_cast<char*>(ram_.data()), static_cast<std::streamsize>(ram_.size()));
}

void Mbc1::SaveRam(std::ofstream& save_file) const
{
    save_file.write(reinterpret_cast<const char*>(ram_.data()),
                    static_cast<std::streamsize>(ram_.size()));
}

// MBC2
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

// MBC3
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
