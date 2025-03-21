#pragma once

#include <utility>

#include "core/sm83/cpu.hpp"

namespace gb
{
class Core
{
public:
    using DrawCallback = std::function<void(const std::array<video::Color, kLcdSize>&)>;

    explicit Core(std::vector<uint8_t> rom_data) : cpu_(std::move(rom_data)) {}

    void RunFrame(const DrawCallback& draw_callback);

    void SetKeyState(Input btn, bool pressed) { GetBus().joypad.SetButton(btn, pressed); }

    memory::Bus& GetBus() { return cpu_.GetBus(); }

private:
    sm83::Cpu cpu_;
};
}  // namespace gb
