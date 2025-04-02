#include "core/core.hpp"

namespace gb
{
Core::Core(const std::filesystem::path& rom_path, DrawCallback draw_cb)
    : cpu_(fs::ReadFile(rom_path)),
      draw_cb_(std::move(draw_cb)),
      rom_path_(rom_path),
      save_path_(fs::kGbcxxDataDir / rom_path.filename().replace_extension(".sav"))
{
    auto& cartridge = cpu_.GetBus().cartridge;
    if (cartridge.has_battery && std::filesystem::exists(save_path_))
    {
        auto save_file = std::ifstream{save_path_, std::ios::in | std::ios::binary};
        cartridge.LoadRam(save_file);
    }
}

Core::~Core()
{
    const auto& cartridge = cpu_.GetBus().cartridge;
    if (cartridge.has_battery) { SaveRam(); }
}

void Core::RunFrame()
{
    constexpr int kCyclesPerFrame = 70224;
    auto& bus = cpu_.GetBus();
    auto& ppu = bus.ppu;

    int this_frame_cycles{};
    while (this_frame_cycles < kCyclesPerFrame)
    {
        ppu.SetShouldDrawFrame(false);

        const uint8_t tcycles = cpu_.Step();
        bus.Tick(tcycles);

        if (ppu.ShouldDrawFrame()) { draw_cb_(ppu.GetLcdBuffer()); }

        this_frame_cycles += tcycles;
    }
}

void Core::SaveRam()
{
    const auto& cartridge = cpu_.GetBus().cartridge;
    std::filesystem::create_directory(fs::kGbcxxDataDir);
    LOG_DEBUG("Core: Saving RAM to {}", save_path_.string());
    auto save_file = std::ofstream{save_path_, std::ios::binary | std::ios::out | std::ios::trunc};
    cartridge.SaveRam(save_file);
}

}  // namespace gb
