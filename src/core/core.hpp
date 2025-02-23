#pragma once

#include <utility>

#include "core/cpu.hpp"

namespace gb
{
class Core
{
public:
    using DrawCallback = std::function<void(const std::array<Color, kLcdSize>&)>;

    explicit Core(std::vector<uint8_t> rom_data) : cpu_(std::move(rom_data)) {}

    void RunFrame(const DrawCallback& draw_callback);

    Bus& GetBus() { return cpu_.GetBus(); }

private:
    Cpu cpu_;
};
}  // namespace gb
