// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Util.hpp>
#include <FX.hpp>

void SDLResultCheck(int result) {
    if(result < 0) throw std::runtime_error(SDL_GetError());
}

void SDLNullCheck(void* ptr) {
    if(!ptr) throw std::runtime_error(SDL_GetError());
}

SDL_Color ColorToSDL(Color color) {
    switch(color) {
        case Color::Black: return {0, 0, 0, 255};
        case Color::White: return {255, 255, 255, 255};
        case Color::Red: return {255, 0, 0, 255};
        case Color::Green: return {0, 255, 0, 255};
        case Color::Gray: return {127, 127, 127};
        case Color::Blue: return {0, 0, 255};
    }
}
