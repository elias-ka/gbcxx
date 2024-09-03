#include "emulator.h"
#include "common/filesystem.h"
#include "common/logging.h"
#include <fmt/format.h>

namespace core
{
    namespace
    {
        std::string get_game_title(const std::vector<uint8_t>& rom)
        {
            std::string title;
            std::ranges::copy_n(rom.begin() + 0x134, 16, std::back_inserter(title));
            return title;
        }

        bool verify_checksum(const std::vector<uint8_t>& rom)
        {
            uint8_t checksum = 0;
            for (uint16_t address = 0x0134; address <= 0x014C; address++)
            {
                checksum = checksum - rom[address] - 1;
            }
            return rom[0x14D] == (checksum & 0x00FF);
        }
    } // namespace

    void emulator::run(const std::filesystem::path& file)
    {
        const std::vector<uint8_t> rom_data = common::fs::read_file_bytes(file);
        if (!verify_checksum(rom_data))
        {
            LOG_CRITICAL("Invalid header checksum, the program won't be run.");
            std::exit(1);
        }

        m_window = std::make_unique<frontend::sdl_window>(
            800, 600, fmt::format("CringeBoy | {}", get_game_title(rom_data)));

        while (m_window->is_open())
        {
            m_window->wait_event();
        }
    }
} // namespace core
