#pragma once

#include "util.hpp"
#include <vector>

namespace cb
{
    struct MbcRomOnly
    {
        explicit MbcRomOnly(std::vector<u8> cartrom)
            : rom(std::move(cartrom))
            , ram(8_KiB)
        {
        }

        std::vector<u8> rom;
        std::vector<u8> ram;
    };

    struct Mbc1
    {
        std::vector<u8> rom;
        std::vector<u8> ram;
        u16 rom_bank{1};
        u16 ram_bank{0};
        bool ram_enabled{};
        bool banking_mode{};
        bool supports_advanced_banking;

        explicit Mbc1(std::vector<u8> cartrom)
            : rom(std::move(cartrom))
            , ram(32_KiB)
            , supports_advanced_banking(rom.size() > 512_KiB)
        {
        }
    };

    using MbcVariant = std::variant<std::monostate, MbcRomOnly, Mbc1>;

    MbcVariant load_cartridge(const std::vector<u8>& cartrom);
} // namespace cb
