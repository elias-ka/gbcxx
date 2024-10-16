#pragma once

#include "core/processor.hpp"
#include "sdl_window.hpp"

namespace cb
{
    class Emulator
    {
    public:
        explicit Emulator(const std::vector<u8>& cartrom);

        void run(SdlWindow* win);

    private:
        Cpu m_cpu;
    };
} // namespace cb
