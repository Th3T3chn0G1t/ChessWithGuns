// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <SoundEffect.hpp>

SoundEffect::SoundEffect(const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    SDLNullCheck(chunk);
    m_Sound.reset(chunk);
}

void SoundEffect::Play() {
    Mix_PlayChannel(-1, m_Sound.get(), 0);
}

void SoundEffect::Loop(Dimension loops) {
    Mix_PlayChannel(-1, m_Sound.get(), loops);
}
