#include "main_app.hpp"

#include <SDL3/SDL.h>

#include "core/util.hpp"

namespace
{
// The emulator output is scaled by this amount
#ifdef __EMSCRIPTEN__
constexpr int kEmuScale = 8;
#else
constexpr int kEmuScale = 4;
#endif
}  // namespace

MainApp::MainApp(const std::filesystem::path& rom_file)
    : core_(rom_file, [this](const gb::video::LcdBuffer& lcd_buf) { viewport_buf_ = lcd_buf; })
{
    if (!SDL_Init(SDL_INIT_VIDEO)) { DIE("Error: SDL_Init(): {}", SDL_GetError()); }
    SDL_CreateWindowAndRenderer("gbcxx", gb::kLcdWidth * kEmuScale, gb::kLcdHeight * kEmuScale,
                                SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) { DIE("Error: SDL_CreateWindowAndRenderer(): {}", SDL_GetError()); }

    viewport_texture_ =
        SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                          gb::kLcdWidth, gb::kLcdHeight);
    if (!viewport_texture_) { DIE("Error: SDL_CreateTexture(): {}", SDL_GetError()); }
    SDL_SetTextureScaleMode(viewport_texture_, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowMinimumSize(window_, gb::kLcdWidth * kEmuScale, gb::kLcdHeight * kEmuScale);
    SDL_ShowWindow(window_);
}

MainApp::~MainApp()
{
    SDL_DestroyTexture(viewport_texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

static gb::Input ScancodeToGbInput(SDL_Scancode scancode)
{
    using enum gb::Input;
    switch (scancode)
    {
    case SDL_SCANCODE_RIGHT: return Right;
    case SDL_SCANCODE_LEFT: return Left;
    case SDL_SCANCODE_UP: return Up;
    case SDL_SCANCODE_DOWN: return Down;
    case SDL_SCANCODE_X: return A;
    case SDL_SCANCODE_Z: return B;
    case SDL_SCANCODE_BACKSPACE: return Select;
    case SDL_SCANCODE_RETURN: return Start;
    default: DIE("Unknown SDL_Scancode to GB input mapping: {}", scancode);
    }
}

void MainApp::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT: quit_ = true; break;
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
        {
            switch (event.key.scancode)
            {
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_X:
            case SDL_SCANCODE_Z:
            case SDL_SCANCODE_BACKSPACE:
            case SDL_SCANCODE_RETURN:
                core_.SetKeyState(ScancodeToGbInput(event.key.scancode),
                                  event.type == SDL_EVENT_KEY_DOWN);
                break;
            default: break;
            }
        }
        break;
        default: break;
        }
    }
}

namespace
{
SDL_FRect CalcRenderViewport(SDL_Window* window)
{
    int window_width;
    int window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    const int scale_x = window_width / gb::kLcdWidth;
    const int scale_y = window_height / gb::kLcdHeight;
    const int scale_factor = std::min(scale_x, scale_y);

    const int target_width = gb::kLcdWidth * scale_factor;
    const int target_height = gb::kLcdHeight * scale_factor;

    const int offset_x = (window_width - target_width) / 2;
    const int offset_y = (window_height - target_height) / 2;

    return {.x = static_cast<float>(offset_x),
            .y = static_cast<float>(offset_y),
            .w = static_cast<float>(target_width),
            .h = static_cast<float>(target_height)};
}
}  // namespace

void MainApp::Step()
{
    PollEvents();
    core_.RunFrame();

    if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) [[unlikely]]
    {
        SDL_Delay(10);
        return;
    }

    SDL_SetRenderDrawColor(renderer_, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer_);

    const SDL_FRect viewport_rect = CalcRenderViewport(window_);
    SDL_UpdateTexture(viewport_texture_, nullptr, viewport_buf_.data(),
                      gb::kLcdWidth * sizeof(gb::video::Color));
    SDL_RenderTexture(renderer_, viewport_texture_, nullptr, &viewport_rect);
    SDL_RenderPresent(renderer_);
}
