// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Context.hpp>

std::random_device Context::RNG{};

Dimension Context::Width;
Dimension Context::Height;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

Context::Context() {
    SDLResultCheck(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS));
    SDLResultCheck(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF));
    SDLResultCheck(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096));

#ifdef __APPLE__
    char path[PATH_MAX + 1] {};
    uint32_t sz = sizeof(path);
    _NSGetExecutablePath(path, &sz);
    std::string cxxpath { path };
    size_t pos = cxxpath.rfind('/');
    std::string dir = cxxpath.substr(0, pos + 1);
    m_ResourcePath = dir + "../Resources";
#endif

    SDL_Window* window = SDL_CreateWindow(Title, Width, Height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
    SDLNullCheck(window);
    m_Window.reset(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(m_Window.get(), nullptr, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDLNullCheck(renderer);
    m_Renderer.reset(renderer);
}

Context::~Context() {
    Mix_CloseAudio();
    SDL_Quit();
}

float Context::SignedRandRange(float range) {
    return ((static_cast<float>(RNG()) / static_cast<float>(std::random_device::max())) * (2 * range)) - range;
}

Dimension Context::SignedRandRange(Dimension range) {
    return static_cast<Dimension>(((static_cast<float>(RNG()) / static_cast<float>(std::random_device::max())) * static_cast<float>(2 * range)) - static_cast<float>(range));
}

Dimension Context::UnsignedRandRange(Dimension range) {
    float mul = static_cast<float>(RNG()) / static_cast<float>(std::random_device::max());
    return static_cast<Dimension>(mul * static_cast<float>(range));
}

bool Context::Update() {
    if(m_ShakeIntensity) m_ShakeIntensity -= UnsignedRandRange(2);
    if(m_ShakeIntensity <= 0) m_ShakeIntensity = 0;

    SDL_RenderPresent(m_Renderer.get());

    SDL_Event event{};
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_EVENT_QUIT: return false;
            case SDL_EVENT_KEY_DOWN: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = true; break;
            case SDL_EVENT_KEY_UP: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = false; break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = true; break;
            case SDL_EVENT_MOUSE_BUTTON_UP: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = false; break;
            default: break;
        }
    }

    return true;
}

void Context::StopSounds() {
    SDLResultCheck(Mix_HaltChannel(-1));
}

void Context::SetColor(Color color) {
    SDL_Color sdl_color = ColorToSDL(color);
    SDLResultCheck(SDL_SetRenderDrawColor(m_Renderer.get(), sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
}

void Context::Clear(Color color) {
    SetColor(color);
    SDLResultCheck(SDL_RenderClear(m_Renderer.get()));
}

void Context::DrawRect(Dimension x, Dimension y, Dimension w, Dimension h, Color color) {
    SetColor(color);

    SDL_FRect rect {static_cast<float>(x + SignedRandRange(m_ShakeIntensity)), static_cast<float>(y + SignedRandRange(m_ShakeIntensity)), static_cast<float>(w), static_cast<float>(h)};
    SDLResultCheck(SDL_RenderFillRect(m_Renderer.get(), &rect));
}

[[nodiscard]] bool Context::IsMouseHeld() const {
    return m_MouseHeld;
}

[[nodiscard]] bool Context::WasMousePressed() {
    bool pressed = IsMouseHeld();
    m_MouseHeld = false;
    return pressed;
}

[[nodiscard]] std::pair<Dimension, Dimension> Context::GetMousePosition() {
    std::pair<float, float> ret;
    SDL_GetMouseState(&ret.first, &ret.second);
    return std::pair<Dimension, Dimension> {static_cast<Dimension>(ret.first), static_cast<Dimension>(ret.second)};
}

void Context::Resize(Dimension width, Dimension height) {
    Width = width;
    Height = height;
    SDL_SetWindowSize(m_Window.get(), Width, Height);
}

bool Context::ChoiceDialog(const std::string& title, const std::string& message) {
    SDL_MessageBoxButtonData buttons[] = {
        {0, 0, "No"},
        {0, 1, "Yes"}
    };
    SDL_MessageBoxData data{ SDL_MESSAGEBOX_INFORMATION, nullptr, title.c_str(), message.c_str(), sizeof(buttons) / sizeof(buttons[0]), buttons, nullptr };
    int result;
    SDLResultCheck(SDL_ShowMessageBox(&data, (int*) &result));
    return result;
}

void Context::Dialog(const std::string& title, const std::string& message) {
    SDLResultCheck(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr));
}

