#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <fmt/format.h>
#include <imgui.h>

#include "core/util.hpp"
#include "main_app.hpp"

int main(int argc, char* argv[])
{
    if constexpr (std::endian::native == std::endian::big) { DIE("Big-endian isn't supported."); }

    using namespace std::string_view_literals;
    spdlog::set_pattern("[%^%l%$] %v");

    const auto args{std::span(argv, static_cast<size_t>(argc))};
    if (args.size() < 2)
    {
        LOG_ERROR("Usage: gbcxx <ROM>");
        return 1;
    }

    const std::filesystem::path rom_file{args[1]};
    if (!std::filesystem::exists(rom_file))
    {
        LOG_ERROR(
            "Failed to open ROM file \"{}\".\nPlease check if the file exists "
            "and the path is "
            "correct.",
            rom_file.string());
        return 1;
    }

#ifndef NDEBUG
    if (args.size() > 2 && std::string_view(args[2]) == "--trace"sv)
    {
        spdlog::set_level(spdlog::level::trace);
    }
    else { spdlog::set_level(spdlog::level::debug); }
#else
    if (args.size() > 2 && std::string_view(args[2]) == "--quiet"sv)
    {
        spdlog::set_level(spdlog::level::off);
    }
    else { spdlog::set_level(spdlog::level::info); }
#endif

    auto app = MainApp{rom_file};
    app.StartApplicationLoop();
}
