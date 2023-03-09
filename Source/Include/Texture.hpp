#pragma once

#include <FX.hpp>

class Context;
class Texture {
private:
    static void Deleter(SDL_Texture* texture) { SDL_DestroyTexture(texture); };
    using Handle = std::unique_ptr<SDLHandle<SDL_Texture>, SDLDestructor<SDL_Texture, Deleter>>;

private:
    Handle m_Texture{};

public:
    static Texture Dummy;
    Dimension m_Width{};
    Dimension m_Height{};
	bool m_Dummy;

public:
    Texture() : m_Dummy(true) {};

    Texture(const std::string& path, Context& ctx);

    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height);
    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height, float rotation);
};

using TextureLoader = ResourceLoader<Texture, 1024>;

struct TextureLoaderWrapper {
    TextureLoader m_Loader;

    explicit TextureLoaderWrapper(TextureLoader loader) : m_Loader(std::move(loader)) {}

    Texture& Get(const std::string& path, Context& ctx) { return m_Loader.Get(path, ctx); }
};
