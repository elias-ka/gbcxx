#include "core/emulator.hpp"
#include "util.hpp"
#include <SDL_timer.h>
#include <fmt/format.h>

namespace cb
{
    namespace
    {
        static constexpr double CLOCK_RATE = 4194304.0;
        static constexpr double CLOCK_CYCLE = 1 / CLOCK_RATE;

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
        const double frame_time = 1.0 / win->refresh_rate();
        const auto cycles_per_frame = static_cast<usz>(frame_time / CLOCK_CYCLE);
        auto& ppu = m_cpu.mmu().ppu();

        LOG_DEBUG("clock_cycle = {}", CLOCK_CYCLE);
        LOG_DEBUG("frame_time = {}", frame_time);
        LOG_DEBUG("cycles_per_frame = {}", cycles_per_frame);

        usz cycles_delta = 0;

        while (win->is_open())
        {
            ppu.clear_should_redraw();
            win->poll_events();

            while (cycles_delta < cycles_per_frame)
            {
                m_cpu.step();
                cycles_delta += m_cpu.cycles_elapsed();
            }

            cycles_delta -= cycles_per_frame;
            if (ppu.should_redraw())
            {
                win->draw(ppu.buffer());
            }
        }
    }

} // namespace cb
