#include <Context.hpp>

std::random_device Context::RNG{};

Dimension Context::Width = 640;
Dimension Context::Height = 480;

Context::Context() {
    SDLResultCheck(SDL_Init(SDL_INIT_EVERYTHING));
    SDLResultCheck(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF));
    SDLResultCheck(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048));

    SDL_Window* window = SDL_CreateWindow(Title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDLNullCheck(window);
    m_Window.reset(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(m_Window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
            case SDL_QUIT: return false;
            case SDL_KEYDOWN: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = true; break;
            case SDL_KEYUP: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = false; break;
            case SDL_MOUSEBUTTONDOWN: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = true; break;
            case SDL_MOUSEBUTTONUP: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = false; break;
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

    SDL_Rect rect {x + SignedRandRange(m_ShakeIntensity), y + SignedRandRange(m_ShakeIntensity), w, h};
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
    std::pair<Dimension, Dimension> ret;
    SDL_GetMouseState(&ret.first, &ret.second);
    return ret;
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

