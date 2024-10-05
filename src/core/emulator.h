#pragma once

#include "core/memory.h"
#include "core/processor.h"
#include "core/video.h"
#include <SDL_events.h>

namespace cb
{
    class window
    {
    public:
        window() = default;
        virtual ~window() = default;

        window(const window&) = delete;
        window(window&&) = delete;
        window operator=(const window&) = delete;
        window operator=(window&&) = delete;

        virtual void draw(const frame_buffer& buf) = 0;
        virtual void poll_events() = 0;

        virtual void set_title(std::string_view title) = 0;
        virtual bool is_open() const = 0;
        virtual int refresh_rate() const = 0;

        virtual void on_keypress(const SDL_KeyboardEvent* event) = 0;
        virtual void on_windowevent(const SDL_WindowEvent* event) = 0;
    };

    class emulator
    {
    public:
        explicit emulator(std::vector<u8> cartrom);
        void run(window* win);

        void tick_components() { m_ppu.tick(); }

    private:
        mmu m_mmu;
        ppu m_ppu;
        cpu m_cpu;
    };
} // namespace cb
