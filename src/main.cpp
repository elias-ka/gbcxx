#include <fmt/format.h>

#include <span>

#include "core/emulator.hpp"
#include "sdl_window.hpp"
#include "util.hpp"

auto main(int argc, char* argv[]) -> int
{
    using namespace std::string_view_literals;

    spdlog::set_pattern("[%^%l%$] %v");

    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        LOG_ERROR("Usage: gbcxx <ROM>");
        return 1;
    }

    const auto rom_file = std::filesystem::path(args[1]);
    if (!std::filesystem::exists(rom_file)) {
        LOG_ERROR("Failed to open ROM file \"{}\". Please check if the file exists and "
                  "the path is correct.",
                  rom_file.string());
        return 1;
    }

#ifndef NDEBUG
    if (args.size() > 2 && std::string_view(args[2]) == "--trace"sv) {
        spdlog::set_level(spdlog::level::trace);
    }
    else {
        spdlog::set_level(spdlog::level::debug);
    }
#endif

    const auto cartrom = gbcxx::fs::read_file(rom_file);
    auto window = gbcxx::Sdl_Window { 160 * 4, 144 * 4, "gbcxx" };
    auto emulator = gbcxx::Emulator { cartrom };
    emulator.run(&window);

    spdlog::shutdown();
}
