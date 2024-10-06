#include "core/emulator.h"
#include "sdl_window.h"
#include "util.h"
#include <fmt/format.h>
#include <span>

int main(int argc, char* argv[])
{

    const auto args = std::span(argv, static_cast<usz>(argc));
    if (args.size() < 2)
    {
        fmt::println("Usage: cringeboy <ROM>");
        return 1;
    }

    const auto rom_file = std::filesystem::path(args[1]);
    if (!std::filesystem::exists(rom_file))
    {
        fmt::println("Error: Failed to open ROM file \"{}\".", rom_file.string());
        fmt::println("Please check if the file exists and the path is correct.");
        return 1;
    }

#ifndef NDEBUG
    if (args.size() > 2 && std::string_view{args[2]} == "--trace")
    {
        cb::logger::instance().set_log_level(cb::log_level::trace);
    }
    else
    {
        cb::logger::instance().set_log_level(cb::log_level::debug);
    }
#endif

    cb::sdl_window window{160 * 4, 144 * 4, "CringeBoy"};
    const std::vector<u8> cartrom = cb::fs::read(rom_file);
    auto emulator = cb::emulator{cartrom};
    emulator.run(&window);
}
