#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace gbcxx {
class Mbc {
public:
    virtual ~Mbc() = default;

    [[nodiscard]] virtual auto read_rom(uint16_t address) const -> uint8_t = 0;
    [[nodiscard]] virtual auto read_ram(uint16_t address) const -> uint8_t = 0;
    virtual auto write_rom(uint16_t address, uint8_t value) -> void = 0;
    virtual auto write_ram(uint16_t address, uint8_t value) -> void = 0;
};

[[nodiscard]] auto make_mbc(std::vector<uint8_t> cartrom) -> std::unique_ptr<Mbc>;
[[nodiscard]] auto count_ram_banks(uint8_t value) -> uint8_t;
[[nodiscard]] auto count_rom_banks(uint8_t value) -> uint8_t;

} // namespace gbcxx
