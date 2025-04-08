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
    : core_(rom_file, [this](const gb::video::LcdBuffer& lcd_buf) { lcd_buf_ = lcd_buf; })
{
    if (!SDL_Init(SDL_INIT_VIDEO)) { DIE("Error: SDL_Init(): {}", SDL_GetError()); }
    SDL_CreateWindowAndRenderer("gbcxx", gb::kLcdWidth * kEmuScale, gb::kLcdHeight * kEmuScale,
                                SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) { DIE("Error: SDL_CreateWindowAndRenderer(): {}", SDL_GetError()); }

    lcd_texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, gb::kLcdWidth, gb::kLcdHeight);
    if (!lcd_texture_) { DIE("Error: SDL_CreateTexture(): {}", SDL_GetError()); }
    SDL_SetTextureScaleMode(lcd_texture_, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowMinimumSize(window_, gb::kLcdWidth * kEmuScale, gb::kLcdHeight * kEmuScale);
    SDL_ShowWindow(window_);
}

MainApp::~MainApp()
{
    SDL_DestroyTexture(lcd_texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

static gb::Input ScancodeToGBInput(SDL_Scancode scancode)
{
    switch (scancode)
    {
    case SDL_SCANCODE_RIGHT: return gb::Input::Right;
    case SDL_SCANCODE_LEFT: return gb::Input::Left;
    case SDL_SCANCODE_UP: return gb::Input::Up;
    case SDL_SCANCODE_DOWN: return gb::Input::Down;
    case SDL_SCANCODE_X: return gb::Input::A;
    case SDL_SCANCODE_Z: return gb::Input::B;
    case SDL_SCANCODE_BACKSPACE: return gb::Input::Select;
    case SDL_SCANCODE_RETURN: return gb::Input::Start;
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
                core_.SetKeyState(ScancodeToGBInput(event.key.scancode),
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
SDL_FRect CalculateIntegerLcdScale(SDL_Window* window)
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

    if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)
    {
        SDL_Delay(10);
        return;
    }

    SDL_SetRenderDrawColor(renderer_, 0x18, 0x18, 0x18, 0xff);
    SDL_RenderClear(renderer_);

    const SDL_FRect lcd_dst_rect = CalculateIntegerLcdScale(window_);
    SDL_UpdateTexture(lcd_texture_, nullptr, lcd_buf_.data(),
                      gb::kLcdWidth * sizeof(gb::video::Color));
    SDL_RenderTexture(renderer_, lcd_texture_, nullptr, &lcd_dst_rect);
    SDL_RenderPresent(renderer_);
}

// namespace
// {
// void OpenRomDialogCallback(void* userdata, const char* const* filelist, int /*filter*/)
// {
//     auto* app = static_cast<MainApp*>(userdata);

//     if (!filelist)
//     {
//         LOG_ERROR("An SDL error occurred: {}", SDL_GetError());
//         return;
//     }

//     if (!*filelist) { return; }

//     if (!std::filesystem::exists(*filelist))
//     {
//         app->ShowErrorMessageBox("The selected ROM file does not exist.");
//         return;
//     }

//     const std::filesystem::path rom_path = *filelist;
//     app->LoadRom(rom_path);
//     LOG_INFO("Loaded ROM {}", rom_path.string());
// }
// }  // namespace
