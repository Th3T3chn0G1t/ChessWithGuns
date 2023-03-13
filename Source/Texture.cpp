// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Texture.hpp>
#include <Context.hpp>

Texture Texture::Dummy{};

Texture::Texture(const std::string& path, Context& ctx) : m_Dummy(false) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    SDLNullCheck(surface);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(ctx.m_Renderer.get(), surface);
    SDLNullCheck(texture);
    m_Texture.reset(texture);

    m_Width = surface->w;
    m_Height = surface->h;

    SDL_DestroySurface(surface);
}

void Texture::Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height) {
    if(!m_Dummy) {
        SDL_FRect src {0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height)};
        SDL_FRect dest {static_cast<float>(x + Context::SignedRandRange(ctx.m_ShakeIntensity)), static_cast<float>(y + Context::SignedRandRange(ctx.m_ShakeIntensity)), static_cast<float>(width), static_cast<float>(height)};
        SDLResultCheck(SDL_RenderTexture(ctx.m_Renderer.get(), m_Texture.get(), &src, &dest));
    }
}

void Texture::Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height, float rotation) {
    if(!m_Dummy) {
        SDL_FRect src {0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height)};
        SDL_FRect dest {static_cast<float>(x + Context::SignedRandRange(ctx.m_ShakeIntensity)), static_cast<float>(y + Context::SignedRandRange(ctx.m_ShakeIntensity)), static_cast<float>(width), static_cast<float>(height)};
        SDLResultCheck(SDL_RenderTextureRotated(ctx.m_Renderer.get(), m_Texture.get(), &src, &dest, rotation, nullptr, SDL_FLIP_NONE));
    }
}

