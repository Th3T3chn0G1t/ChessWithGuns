// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <Context.hpp>
#include <Texture.hpp>

class Tickbox {
private:
	Dimension m_X;
	Dimension m_Y;
    Dimension m_UnitSize;

	std::reference_wrapper<Texture> m_TextureOn;
	std::reference_wrapper<Texture> m_TextureOff;

public:
	bool m_State;

	Tickbox(Dimension x, Dimension y, Dimension unit_size, bool state, const std::string& on_path, const std::string& off_path, Context& ctx, TextureLoader& loader) : m_X(x), m_Y(y), m_UnitSize(unit_size), m_State(state), m_TextureOn(loader.Get(on_path, ctx)), m_TextureOff(loader.Get(off_path, ctx)) {}
	bool Update(Context& ctx, bool mouse_down);
};

enum class UIResult {
	None,
	Hover,
	Click
};

class Button {
private:
	std::reference_wrapper<Texture> m_Texture;

public:
	Dimension m_X;
	Dimension m_Y;
	Dimension m_Width;
	Dimension m_Height;

	float m_Rotation { 0.0f };

	Button(Dimension x, Dimension y, Dimension width, Dimension height, const std::string& path, Context& ctx, TextureLoader& loader) : m_X(x), m_Y(y), m_Width(width), m_Height(height), m_Texture(loader.Get(path, ctx)) {}
	UIResult Update(Context& ctx, bool mouse_down);
};

class ArrowSelect {
private:
    Button m_Left;
    Button m_Right;
	std::vector<std::reference_wrapper<Texture>> m_ItemTextures;

public:
    Dimension m_Current;

	Dimension m_X;
	Dimension m_Y;
	Dimension m_UnitSize;

    ArrowSelect(Dimension x, Dimension y, Dimension unit_size, Span<std::string> paths, Context& ctx, TextureLoader& loader);
    bool Update(Context& ctx, bool mouse_down);
};
