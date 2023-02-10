#include <SoundEffect.hpp>

SoundEffect::SoundEffect(const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    SDLNullCheck(chunk);
    m_Sound.reset(chunk);
}

void SoundEffect::Play() {
    Mix_PlayChannel(-1, m_Sound.get(), 0);
}
