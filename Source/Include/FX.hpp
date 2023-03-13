// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_mixer.h>

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
