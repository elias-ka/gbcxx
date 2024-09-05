#include "core/emulator.h"
#include <fmt/format.h>
#include <span>

int main(int argc, char* argv[])
{
    const auto args = std::span(argv, static_cast<size_t>(argc));

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

    auto emulator = core::emulator{};
    emulator.run(rom);
}
