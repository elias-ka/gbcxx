#pragma once

#include <array>
#include <cstdint>
#include <fstream>
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

class Mbc0 final : public Mbc
{
public:
    explicit Mbc0(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
};

class Mbc1 final : public Mbc
{
public:
    explicit Mbc1(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
    size_t nr_rom_banks_{0};
    size_t nr_ram_banks_{0};
    std::vector<uint8_t> ram_;
    size_t rom_bank_{1};
    size_t ram_bank_{0};
    uint8_t banking_mode_{0};
    bool ram_enabled_{false};
};

class Mbc2 : public Mbc
{
public:
    explicit Mbc2(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
    std::array<uint8_t, 512> ram_;
    size_t rom_bank_{1};
    bool ram_enabled_{false};
};

class Mbc3 : public Mbc
{
public:
    explicit Mbc3(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;
    size_t rom_bank_{1};
    size_t ram_bank_{0};
    bool ram_enabled_{false};
};
}  // namespace gb::memory
