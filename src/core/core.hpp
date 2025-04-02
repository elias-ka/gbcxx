#pragma once

#include "core/sm83/cpu.hpp"

namespace gb
{
class Core
{
public:
    using DrawCallback = std::function<void(const std::array<video::Color, kLcdSize>&)>;

    explicit Core(const std::filesystem::path& rom_path, DrawCallback draw_cb);
    ~Core();

    memory::Bus& GetBus() { return cpu_.GetBus(); }

    void RunFrame();
    void SaveRam();
    void SetKeyState(Input btn, bool pressed) { GetBus().joypad.SetButton(btn, pressed); }

private:
    sm83::Cpu cpu_;
    DrawCallback draw_cb_;
    std::filesystem::path rom_path_;
    std::filesystem::path save_path_;
};
}  // namespace gb
