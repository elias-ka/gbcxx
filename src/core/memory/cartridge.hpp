#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace gb::memory
{
class Mbc
{
public:
    virtual ~Mbc() = default;

    [[nodiscard]] virtual uint8_t ReadRom(uint16_t addr) const = 0;
    [[nodiscard]] virtual uint8_t ReadRam(uint16_t addr) const = 0;

    virtual void WriteRom(uint16_t addr, uint8_t val) = 0;
    virtual void WriteRam(uint16_t addr, uint8_t val) = 0;

    virtual void LoadRam(std::ifstream& save_file) = 0;
    virtual void SaveRam(std::ofstream& save_file) const = 0;
};

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

[[nodiscard]] size_t CountRamBanks(uint8_t val);
[[nodiscard]] size_t CountRomBanks(uint8_t val);
}  // namespace gb::memory
