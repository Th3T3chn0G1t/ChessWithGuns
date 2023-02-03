#pragma once

#include <Util.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

enum class Color {
    Black,
    White,
    Red,
    Green,
    Gray,
    Blue
};

template<class T>
class SDLHandle {
public:
    T* m_UnderlyingHandle{nullptr};

public:
    SDLHandle() = default;
    SDLHandle(T* handle) : m_UnderlyingHandle(handle) {}

    operator T*() const { return m_UnderlyingHandle; }
};

template<class T, void (*func)(T*)>
class SDLDestructor {
public:
    using pointer  = SDLHandle<T>;

public:
    void operator()(SDLHandle<T> handle) const {
        func(handle);
    }
};

void SDLResultCheck(int result);
void SDLNullCheck(void* ptr);
SDL_Color ColorToSDL(Color color);
