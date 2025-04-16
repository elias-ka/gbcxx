#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "core/memory/mbc.hpp"

namespace gb::memory
{
struct Cartridge
{
    std::unique_ptr<Mbc> mbc;
    bool has_battery{};

    static Cartridge FromRom(std::vector<uint8_t> rom_data);

    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const;
    void WriteByte(uint16_t addr, uint8_t val) const;

    void LoadRam(std::ifstream& save_file) const;
    void SaveRam(std::ofstream& save_file) const;
};
}  // namespace gb::memory
