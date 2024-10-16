#include "core/emulator.h"
#include "sdl_window.h"
#include "util.h"
#include <fmt/format.h>
#include <span>

int main(int argc, char* argv[])
{
    spdlog::set_pattern("[%^%l%$] %v");

    const auto args = std::span(argv, static_cast<usz>(argc));
    if (args.size() < 2)
    {
        LOG_ERROR("Usage: cringeboy <ROM>");
        return 1;
    }

    const auto rom_file = std::filesystem::path(args[1]);
    if (!std::filesystem::exists(rom_file))
    {
        LOG_ERROR("Failed to open ROM file \"{}\". Please check if the file exists and the "
                  "path is correct.",
                  rom_file.string());
        return 1;
    }

#ifndef NDEBUG
    if (args.size() > 2 && std::string_view{args[2]} == "--trace")
        spdlog::set_level(spdlog::level::trace);
    else
        spdlog::set_level(spdlog::level::debug);
#endif

    cb::SdlWindow window{160 * 4, 144 * 4, "CringeBoy"};
    const std::vector<u8> cartrom = cb::fs::read(rom_file);
    auto emulator = cb::Emulator{cartrom};
    emulator.run(&window);

    spdlog::shutdown();
}
