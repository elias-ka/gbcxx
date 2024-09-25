#include "emulator.h"
#include "../util.h"
#include <SDL_timer.h>
#include <fmt/format.h>

namespace cb
{
    void emulator::run(const std::filesystem::path& file, window* win)
    {
        const std::vector<u8> rom_data = cb::fs::read(file);
        const bool checksum_matches = [&]
        {
            u8 checksum = 0;
            for (u16 address = 0x0134; address <= 0x014C; address++)
            {
                checksum -= rom_data[address] + 1;
            }
            return rom_data[0x14D] == checksum;
        }();

        if (!checksum_matches)
        {
            LOG_ERROR("Invalid header checksum, the program won't be run.");
            std::exit(1);
        }

        const double clock_cycle_us = 1.0 / 4194304.0;
        const double frame_time_us = 1.0 / win->refresh_rate();
        const auto cycles_per_frame = static_cast<usz>(frame_time_us / clock_cycle_us);
        LOG_DEBUG("clock_cycle_us = {}", clock_cycle_us);
        LOG_DEBUG("frame_time_us = {}", frame_time_us);
        LOG_DEBUG("cycles_per_frame = {}", cycles_per_frame);

        usz cycles_delta = 0;

        while (win->is_open())
        {
            m_mmu.ppu().clear_should_redraw();
            win->poll_events();

            while (cycles_delta < cycles_per_frame)
            {
                m_cpu.step(m_mmu);
                cycles_delta += m_cpu.cycles_elapsed();
            }

            cycles_delta -= cycles_per_frame;
            if (m_mmu.ppu().should_redraw())
            {
                win->draw(m_mmu.ppu().buffer());
            }
        }
    }

} // namespace cb
