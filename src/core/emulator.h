#pragma once

#include "core/memory.h"
#include "core/processor.h"
#include "core/video.h"
#include "sdl_window.h"

namespace cb
{
    class emulator
    {
    public:
        explicit emulator(std::vector<u8> cartrom);
        void run(sdl_window* win);

        void tick_components() { m_ppu.tick(); }

    private:
        mmu m_mmu;
        ppu m_ppu;
        cpu m_cpu;
    };
} // namespace cb
