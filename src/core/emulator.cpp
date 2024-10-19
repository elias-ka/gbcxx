#include "core/emulator.hpp"
#include "util.hpp"
#include <SDL2/SDL_timer.h>
#include <fmt/format.h>

namespace cb
{
    namespace
    {
        bool checksum_matches(const std::vector<u8>& cartrom)
        {
            u8 checksum = 0;
            for (u16 address = 0x0134; address <= 0x014C; address++)
            {
                checksum -= cartrom[address] + 1;
            }
            return cartrom[0x14D] == checksum;
        }

        const std::vector<u8>& verify_checksum(const std::vector<u8>& cartrom)
        {
            if (!checksum_matches(cartrom))
                DIE("Invalid header checksum, the program won't be run.");

            return cartrom;
        }
    } // namespace

    Emulator::Emulator(const std::vector<u8>& cartrom)
        : m_cpu(load_cartridge(verify_checksum(cartrom)))
    {
    }

    void Emulator::run(SdlWindow* win)
    {
        const f64 CLOCK_RATE = 4194304.0;
        const f64 CLOCK_CYCLE = 1 / CLOCK_RATE;
        const f64 frame_time = 1.0 / win->refresh_rate();
        const usz cycles_per_frame = static_cast<usz>(frame_time / CLOCK_CYCLE);
        auto& ppu = m_cpu.mmu().ppu();

        LOG_DEBUG("clock_cycle = {}", CLOCK_CYCLE);
        LOG_DEBUG("frame_time = {}", frame_time);
        LOG_DEBUG("cycles_per_frame = {}", cycles_per_frame);

        while (win->is_open())
        {
            usz this_frame_cycles = 0;

            ppu.clear_should_redraw();
            win->poll_events();

            while (this_frame_cycles < cycles_per_frame)
            {
                m_cpu.step();
                this_frame_cycles += m_cpu.cycles_elapsed();
            }

            this_frame_cycles -= cycles_per_frame;
            m_cpu.subtract_cycles(this_frame_cycles);

            if (ppu.should_redraw())
            {
                win->draw(ppu.buffer());
            }
        }
    }

} // namespace cb
