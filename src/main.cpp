#include "core/emulator.h"
#include "sdl_window.h"
#include "util.h"
#include <fmt/format.h>
#include <span>

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    cb::logger::instance().set_log_level(cb::log_level::debug);
#endif
    const auto args = std::span(argv, static_cast<usz>(argc));
    if (args.size() != 2)
    {
        fmt::println("Usage: cringeboy <ROM>");
        return 1;
    }

    const auto rom = std::filesystem::path(args[1]);
    if (!std::filesystem::exists(rom))
    {
        fmt::println("Error: Failed to open ROM file \"{}\".", rom.string());
        fmt::println("Please check if the file exists and the path is correct.");
        return 1;
    }

    const auto ext = rom.extension();
    if (ext != ".dmg" && ext != ".gb" && ext != ".gbc" && ext != ".bin")
    {
        fmt::println("Error: Invalid file extension \"{}\".", ext.string());
        fmt::println("Supported extensions are:\n  - .dmg\n  - .bin\n  - .gb\n  - .gbc");
        return 1;
    }

    std::unique_ptr<cb::window> window =
        std::make_unique<frontend::sdl_window>(160, 144, "CringeBoy");
    auto emulator = cb::emulator{};
    emulator.run(rom, window.get());
}
