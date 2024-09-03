#pragma once

#include "core/memory/mmu.h"
#include "sdl_window.h"
#include <filesystem>

namespace core
{
    class emulator
    {
    public:
        void run(const std::filesystem::path& file);

    private:
        memory::mmu m_mmu;
        std::unique_ptr<frontend::sdl_window> m_window;
    };
} // namespace core
