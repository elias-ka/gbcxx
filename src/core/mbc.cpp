#include "core/mbc.hpp"

#include "core/mbc/mbc0.hpp"
#include "core/mbc/mbc1.hpp"

namespace gbcxx {
std::unique_ptr<Mbc> make_mbc(std::vector<u8> cartrom) {
  {
    switch (cartrom.at(0x147)) {
      case 0x00: {
        LOG_INFO("Selected MBC0");
        return std::make_unique<Mbc0>(std::move(cartrom));
      }
      case 0x01:
      case 0x02:
      case 0x03: {
        LOG_INFO("Selected MBC1");
        return std::make_unique<Mbc1>(std::move(cartrom));
      }
    }
    DIE("Unimplemented cartridge type {:#02x}", cartrom[0x0147]);
  }
}

u8 count_ram_banks(u8 value) {
  switch (value) {
    case 1:
    case 2: return 1;
    case 3: return 4;
    case 4: return 16;
    case 5: return 8;
    default: return 0;
  }
}

u8 count_rom_banks(u8 value) {
  if (value < 9) {
    return static_cast<u8>(2 << value);
  }
  return 0;
}

}  // namespace gbcxx
