#include "core/mbc/mbc0.hpp"

namespace gb
{
Mbc0::Mbc0(std::vector<uint8_t> cartrom) : rom_(std::move(cartrom)) {}

uint8_t Mbc0::ReadRom(uint16_t addr) const
{
    return rom_[addr];
}
uint8_t Mbc0::ReadRam(uint16_t /*address*/) const
{
    return 0;
}
void Mbc0::WriteRom(uint16_t /*address*/, uint8_t /*value*/) {}
void Mbc0::WriteRam(uint16_t /*address*/, uint8_t /*value*/) {}
void Mbc0::LoadRam(std::vector<uint8_t> /*ram*/) {}

}  // namespace gb
