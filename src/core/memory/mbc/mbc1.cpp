#include "core/memory/mbc/mbc1.hpp"

#include "core/util.hpp"

namespace gb::memory
{
Mbc1::Mbc1(std::vector<uint8_t> cartrom)
    : rom_(std::move(cartrom)),
      nr_rom_banks_(CountRomBanks(rom_[0x148])),
      nr_ram_banks_(CountRamBanks(rom_[0x149])),
      ram_(nr_rom_banks_ * 0x2000)
{
}

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

}  // namespace gb::memory
