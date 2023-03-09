#pragma once

#include <Util.hpp>
#include <Context.hpp>
#include <Texture.hpp>

class Tickbox {
private:
	Dimension m_X;
	Dimension m_Y;

	std::reference_wrapper<Texture> m_TextureOn;
	std::reference_wrapper<Texture> m_TextureOff;

public:
	bool m_State;

	Tickbox(Dimension x, Dimension y, bool state) : m_X(x), m_Y(y), m_State(state), m_TextureOn(Texture::Dummy), m_TextureOff(Texture::Dummy) {}
	Tickbox(Dimension x, Dimension y, bool state, const std::string& on_path, const std::string& off_path, Context& ctx, TextureLoader& loader) : m_X(x), m_Y(y), m_State(state), m_TextureOn(loader.Get(on_path, ctx)), m_TextureOff(loader.Get(off_path, ctx)) {}
	bool Update(Context& ctx, bool mouse_down);
};

enum class UIResult {
	None,
	Hover,
	Click
};

class Button {
private:
	Dimension m_X;
	Dimension m_Y;
	Dimension m_Width;
	Dimension m_Height;

	std::reference_wrapper<Texture> m_Texture;

public:
	float m_Rotation { 0.0f };

	Button(Dimension x, Dimension y, Dimension width, Dimension height, const std::string& path, Context& ctx, TextureLoader& loader) : m_X(x), m_Y(y), m_Width(width), m_Height(height), m_Texture(loader.Get(path, ctx)) {}
	UIResult Update(Context& ctx, bool mouse_down);
};
