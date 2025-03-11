#include "core/core.hpp"

namespace gb
{
void Core::RunFrame(const DrawCallback& draw_callback)
{
    constexpr int kCyclesPerFrame = 70224;
    auto& bus = cpu_.GetBus();

    int this_frame_cycles{};
    while (this_frame_cycles < kCyclesPerFrame)
    {
        bus.ppu.SetShouldDrawFrame(false);

        const uint8_t tcycles = cpu_.Step();
        bus.Tick(tcycles);

        if (bus.ppu.ShouldDrawFrame())
        {
            draw_callback(bus.ppu.GetLcdBuffer());
        }

        this_frame_cycles += tcycles;
    }
}

}  // namespace gb
