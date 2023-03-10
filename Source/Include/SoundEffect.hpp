// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <FX.hpp>

class SoundEffect {
private:
    static void Deleter(Mix_Chunk* chunk) { Mix_FreeChunk(chunk); };
    using Handle = std::unique_ptr<SDLHandle<Mix_Chunk>, SDLDestructor<Mix_Chunk, Deleter>>;

    Handle m_Sound;

public:
    SoundEffect() = default;

    SoundEffect(const std::string& path);

    void Play();
    void Loop(Dimension loops);
};
using SoundEffectLoader = ResourceLoader<SoundEffect, 1024>;
