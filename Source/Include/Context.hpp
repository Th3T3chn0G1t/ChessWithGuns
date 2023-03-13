// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <FX.hpp>
#include <CWG.hpp>

class Context {
private:
    static void WindowDeleter(SDL_Window* window) { SDL_DestroyWindow(window); };
    using WindowHandle = std::unique_ptr<SDLHandle<SDL_Window>, SDLDestructor<SDL_Window, WindowDeleter>>;

    static void RendererDeleter(SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); };
    using RendererHandle = std::unique_ptr<SDLHandle<SDL_Renderer>, SDLDestructor<SDL_Renderer, RendererDeleter>>;

    static std::random_device RNG;

    static constexpr const char Title[] = "Chess with Guns";
public:
    static constexpr Dimension SidebarWidth = 192;
    static Dimension Width;
    static Dimension Height;

    Dimension m_ShakeIntensity = 0;
private:
    WindowHandle m_Window;
    RendererHandle m_Renderer;

    std::unordered_map<SDL_KeyCode, bool> m_KeyStates;
    bool m_MouseHeld{};

    friend class Texture;

public:
    Context();
    ~Context();

    static float SignedRandRange(float range);
    static Dimension SignedRandRange(Dimension range);
    static Dimension UnsignedRandRange(Dimension range);

    bool Update();

    void SetColor(Color color);
    void Clear(Color color);
    void DrawRect(Dimension x, Dimension y, Dimension w, Dimension h, Color color);
    [[nodiscard]] bool IsMouseHeld() const;
    [[nodiscard]] bool WasMousePressed();
    [[nodiscard]] static std::pair<Dimension, Dimension> GetMousePosition();
    void Resize(Dimension width, Dimension height);

	static void StopSounds();

    template<class T>
    using DialogChoices = std::vector<std::pair<T, std::string>>;

    template<class T>
    static T ChoiceDialog(DialogChoices<T> options, const std::string& title, const std::string& message) {
        auto* buttons = new SDL_MessageBoxButtonData[options.size()];
        for(Dimension i = 0; i < options.size(); ++i) {
            buttons[i] = SDL_MessageBoxButtonData{ 0, (int) options[i].first, options[i].second.c_str() };
        }

        SDL_MessageBoxData data{ SDL_MESSAGEBOX_INFORMATION, nullptr, title.c_str(), message.c_str(), static_cast<int>(options.size()), buttons, nullptr };
        T result;
        SDLResultCheck(SDL_ShowMessageBox(&data, (int*) &result));

        delete[] buttons;
        return result;
    }

    static bool ChoiceDialog(const std::string& title, const std::string& message);
    static void Dialog(const std::string& title, const std::string& message);
};
