// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <Context.hpp>
#include <Texture.hpp>

enum class UIResult {
	None,
	Hover,
	Click
};

class UIElement;
class UIElement {
public:
	Dimension m_X;
	Dimension m_Y;

private:
    bool m_Vertical { true }; // TODO
    std::vector<UIElement> m_Children;

public:
    UIElement(Dimension x, Dimension y) : m_X(x), m_Y(y) {}

    virtual UIResult Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) = 0;
    virtual Dimension Right() = 0;
    virtual Dimension Bottom() = 0;

    template<class... Args>
    void AddChild(Args&&... args) {
        m_Children.emplace_back(std::forward<Args>(args)...);
    }

protected:
    void UpdateChildren(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy);
};

class Tickbox : public UIElement {
private:
    Dimension m_UnitSize;

	std::reference_wrapper<Texture> m_TextureOn;
	std::reference_wrapper<Texture> m_TextureOff;

public:
	bool m_State;

	Tickbox(Dimension x, Dimension y, Dimension unit_size, bool state, const std::string& on_path, const std::string& off_path, Context& ctx, TextureLoader& loader) : UIElement(x, y), m_UnitSize(unit_size), m_State(state), m_TextureOn(loader.Get(on_path, ctx)), m_TextureOff(loader.Get(off_path, ctx)) {}
    UIResult Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) override;

    Dimension Right() override { return m_X + m_UnitSize; }
    Dimension Bottom() override { return m_Y + m_UnitSize; }
};

class Button : public UIElement {
private:
	std::reference_wrapper<Texture> m_Texture;

public:
	Dimension m_Width;
	Dimension m_Height;

	float m_Rotation { 0.0f };

	Button(Dimension x, Dimension y, Dimension width, Dimension height, const std::string& path, Context& ctx, TextureLoader& loader) : UIElement(x, y), m_Width(width), m_Height(height), m_Texture(loader.Get(path, ctx)) {}
	UIResult Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) override;

    Dimension Right() override { return m_X + m_Width; }
    Dimension Bottom() override { return m_Y + m_Height; }
};

class ArrowSelect : public UIElement {
private:
    Button m_Left;
    Button m_Right;
	std::vector<std::reference_wrapper<Texture>> m_ItemTextures;

public:
    Dimension m_UnitSize;

    Dimension m_Current;

    ArrowSelect(Dimension x, Dimension y, Dimension unit_size, Span<const std::string> paths, Context& ctx, TextureLoader& loader);
    UIResult Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) override;

    Dimension Right() override { return m_X + (3 * m_UnitSize); }
    Dimension Bottom() override { return m_Y + m_UnitSize; }
};
