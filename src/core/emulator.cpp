#include "emulator.h"
#include "../util.h"
#include <SDL_timer.h>
#include <fmt/format.h>

namespace cb
{
    namespace
    {
        static constexpr double clock_rate = 4194304.0;
        static constexpr double clock_cycle = 1 / clock_rate;

        bool checksum_matches(std::span<const u8> cartrom)
        {
            u8 checksum = 0;
            for (u16 address = 0x0134; address <= 0x014C; address++)
            {
                checksum -= cartrom[address] + 1;
            }
            return cartrom[0x14D] == checksum;
        }

        std::vector<u8> verify_checksum(std::vector<u8>& cartrom)
        {
            if (!checksum_matches(cartrom))
            {
                LOG_ERROR("Invalid header checksum, the program won't be run.");
                std::exit(1);
            }
            return cartrom;
        }
    } // namespace

    emulator::emulator(std::vector<u8> cartrom)
        : m_mmu(cartridge{verify_checksum(cartrom)})
        , m_cpu(&m_mmu)
    {
    }

    void emulator::run(window* win)
    {
        const double frame_time = 1.0 / win->refresh_rate();
        const auto cycles_per_frame = static_cast<usz>(frame_time / clock_cycle);

        LOG_DEBUG("clock_cycle = {}", clock_cycle);
        LOG_DEBUG("frame_time = {}", frame_time);
        LOG_DEBUG("cycles_per_frame = {}", cycles_per_frame);

        usz cycles_delta = 0;

        while (win->is_open())
        {
            m_mmu.ppu().clear_should_redraw();
            win->poll_events();

            while (cycles_delta < cycles_per_frame)
            {
                m_cpu.step();
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
