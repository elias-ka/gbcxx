#include "core/mbc/mbc1.hpp"

#include "core/util.hpp"

namespace gb
{
Mbc1::Mbc1(std::vector<uint8_t> cartrom)
    : rom_(std::move(cartrom)),
      nr_rom_banks_(CountRomBanks(rom_.at(0x148))),
      nr_ram_banks_(CountRamBanks(rom_.at(0x149))),
      ram_(static_cast<size_t>(nr_rom_banks_ * 0x2000))
{
}

uint8_t Mbc1::ReadRom(uint16_t addr) const
{
    const uint8_t bank =
        (addr < 0x4000) ? ((banking_mode_ == 0) ? 0 : rom_bank_ & 0b0001'1111) : rom_bank_;
    const uint16_t offset = (bank * 0x4000) | (addr & 0x3fff);

    if (offset >= rom_.size())
    {
        DIE("MBC1: Invalid read {:X} (bank {:X})", addr, bank);
    }

    return rom_[offset];
}

uint8_t Mbc1::ReadRam(uint16_t addr) const
{
    if (!ram_enabled_)
    {
        return 0xff;
    }

    const uint8_t ram_bank = banking_mode_ == 1 ? ram_bank_ : 0;
    return ram_.at((ram_bank * 0x2000) | (addr & 0x1fff));
}

void Mbc1::WriteRom(uint16_t addr, uint8_t val)
{
    if (0x0000 <= addr && addr <= 0x1fff)
    {
        ram_enabled_ = ((val & 0xf) == 0xa);
    }
    else if (0x2000 <= addr && addr <= 0x3fff)
    {
        const uint8_t lower_bits = (val & 0x1f) == 0 ? 1 : (val & 0x1f);
        rom_bank_ = ((rom_bank_ & 0x60) | lower_bits) % nr_rom_banks_;
    }
    else if (0x4000 <= addr && addr <= 0x5fff)
    {
        if (nr_rom_banks_ > 32)
        {
            const uint8_t upper_bits = (val & 0x03) % (nr_rom_banks_ >> 5);
            rom_bank_ = static_cast<uint8_t>((rom_bank_ & 0x1f) | (upper_bits << 5));
        }
        if (nr_ram_banks_ > 1)
        {
            ram_bank_ = val & 0x03;
        }
    }
    else if (0x6000 <= addr && addr <= 0x7fff)
    {
        banking_mode_ = val & 0x01;
    }
    else
    {
        DIE("MBC1: Unmapped ROM write {:X} <- {:X}", addr, val);
    }
}

void Mbc1::WriteRam(uint16_t addr, uint8_t val)
{
    if (!ram_enabled_)
    {
        LOG_WARN("MBC1: RAM is disabled, ignoring write {:X} <- {:X}", addr, val);
        return;
    }

    const uint8_t ram_bank = banking_mode_ == 1 ? ram_bank_ : 0;
    ram_.at((ram_bank * 0x2000) + (addr & 0x1fff)) = val;
}
}  // namespace gb
