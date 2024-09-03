#include "emulator.h"
#include <fmt/format.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fmt::println("Usage: cringeboy <ROM>");
        return 1;
    }

    const std::filesystem::path rom_path = argv[1];

    if (!std::filesystem::exists(rom_path))
    {
        fmt::println("Failed to open ROM '{}'.", rom_path.string());
        fmt::println("Ensure that the specified file exists.");
        return 1;
    }

    if (!rom_path.has_extension() ||
        (rom_path.extension() != ".dmg" && rom_path.extension() != ".gb" &&
         rom_path.extension() != ".gbc" && rom_path.extension() != ".bin"))
    {
        fmt::println("Invalid file extension: {}.", rom_path.extension().string());
        fmt::println("Allowed extensions are .dmg, .bin, .gb, and .gbc.");
        return 1;
    }

    auto emulator = core::emulator{};
    emulator.run(argv[1]);
}
