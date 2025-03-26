#include "main_app.hpp"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <imgui.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>

#include <utility>
#endif

#include "core/util.hpp"
#include "font_data.hpp"

namespace
{
// The emulator output is scaled by this amount
constexpr int kEmuScale = 4;
}  // namespace

MainApp::MainApp(std::vector<uint8_t> rom_data) : core_(std::move(rom_data))
{
    if (!SDL_Init(SDL_INIT_VIDEO)) { DIE("Error: SDL_Init(): {}", SDL_GetError()); }
    SDL_CreateWindowAndRenderer("gbcxx", gb::kLcdWidth, gb::kLcdHeight,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE,
                                &window_, &renderer_);
    if (!window_ || !renderer_) { DIE("Error: SDL_CreateWindowAndRenderer(): {}", SDL_GetError()); }

    lcd_texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, gb::kLcdWidth, gb::kLcdHeight);
    if (!lcd_texture_) { DIE("Error: SDL_CreateTexture(): {}", SDL_GetError()); }
    SDL_SetTextureScaleMode(lcd_texture_, SDL_SCALEMODE_NEAREST);

    vram_bg_texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING, 256, 256);
    if (!vram_bg_texture_) { DIE("Error: SDL_CreateTexture(): {}", SDL_GetError()); }
    SDL_SetTextureScaleMode(vram_bg_texture_, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowMinimumSize(window_, gb::kLcdWidth * kEmuScale, gb::kLcdHeight * kEmuScale);
    SDL_ShowWindow(window_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0F;
    style.FrameRounding = 0.0F;
    style.ScrollbarRounding = 0.0F;
    style.GrabRounding = 0.0F;
    style.TabRounding = 0.0F;

    font_body_ = io.Fonts->AddFontFromMemoryCompressedTTF(kRobotoMonoRegularCompressedData,
                                                          kRobotoMonoRegularCompressedSize, 18.0F);
    font_body_ = io.Fonts->AddFontFromMemoryCompressedTTF(kRobotoRegularCompressedData,
                                                          kRobotoRegularCompressedSize, 18.0F);

    io.FontDefault = font_body_;

    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);
}

MainApp::~MainApp()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

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
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                                             event.window.windowID == SDL_GetWindowID(window_)))
        {
            quit_ = true;
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
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
    }
}

namespace
{
SDL_FRect CalculateIntegerLcdScale(SDL_Window* window, float menu_bar_height)
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
            .y = static_cast<float>(offset_y) + menu_bar_height,
            .w = static_cast<float>(target_width),
            .h = static_cast<float>(target_height) - menu_bar_height};
}
}  // namespace

void MainApp::StartApplicationLoop()
{
    while (!quit_)
    {
        PollEvents();
        core_.RunFrame([this](const gb::video::LcdBuffer& lcd_buf) { lcd_buf_ = lcd_buf; });

        if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);
        MainMenu();

        if (show_imgui_demo_) { ImGui::ShowDemoWindow(&show_imgui_demo_); }

        if (show_vram_debug_window_)
        {
            if (ImGui::Begin("VRAM Viewer", &show_vram_debug_window_))
            {
                ImGui::BeginTabBar("##vram_tabs");
                if (ImGui::BeginTabItem("Background"))
                {
                    core_.GetBus().ppu.DrawBackgroundTileMap(vram_bg_fb_);
                    SDL_UpdateTexture(vram_bg_texture_, nullptr, vram_bg_fb_.data(),
                                      256 * sizeof(gb::video::Color));
                    ImGui::Image(reinterpret_cast<ImTextureID>(vram_bg_texture_), {256, 256});
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Window"))
                {
                    core_.GetBus().ppu.DrawWindowTileMap(vram_bg_fb_);
                    SDL_UpdateTexture(vram_window_texture_, nullptr, vram_window_fb_.data(),
                                      256 * sizeof(gb::video::Color));
                    ImGui::Image(reinterpret_cast<ImTextureID>(vram_window_texture_), {256, 256});
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        ImGui::Render();
        SDL_SetRenderScale(renderer_, ImGui::GetIO().DisplayFramebufferScale.x,
                           ImGui::GetIO().DisplayFramebufferScale.y);

        SDL_SetRenderDrawColor(renderer_, 0x18, 0x18, 0x18, 0xff);
        SDL_RenderClear(renderer_);

        const SDL_FRect lcd_dst_rect = CalculateIntegerLcdScale(window_, menu_bar_height_);
        SDL_UpdateTexture(lcd_texture_, nullptr, lcd_buf_.data(),
                          gb::kLcdWidth * sizeof(gb::video::Color));
        SDL_RenderTexture(renderer_, lcd_texture_, nullptr, &lcd_dst_rect);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
        SDL_RenderPresent(renderer_);
    }
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

//     if (!*filelist) return;

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

void MainApp::MainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        menu_bar_height_ = ImGui::GetWindowHeight();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open ROM..."))
            {
                // static const std::array<SDL_DialogFileFilter, 1> kFilters = {{
                //     {.name = "Game Boy (.gb, .dmg)", .pattern = "gb;dmg"},
                // }};
                // SDL_ShowOpenFileDialog(&OpenRomDialogCallback, /*userdata=*/this, window_,
                //                        kFilters.data(), kFilters.size(),
                //                        /*default_location=*/nullptr, /*allow_many=*/false);
            }

            if (ImGui::MenuItem("Show ImGui Demo", nullptr, show_imgui_demo_))
            {
                show_imgui_demo_ = !show_imgui_demo_;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) { quit_ = true; }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("VRAM", nullptr, show_vram_debug_window_))
            {
                show_vram_debug_window_ = !show_vram_debug_window_;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void MainApp::ShowErrorMessageBox(const std::string& message) const
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), window_);
}
