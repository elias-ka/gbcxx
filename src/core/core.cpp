#include "core/core.hpp"

namespace gb
{
void Core::RunFrame(const DrawCallback& draw_callback)
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

        if (ppu.ShouldDrawFrame())
        {
            draw_callback(ppu.GetLcdBuffer());
        }

        this_frame_cycles += tcycles;
    }
}

}  // namespace gb
