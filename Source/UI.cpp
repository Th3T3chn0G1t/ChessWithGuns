// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <UI.hpp>

void UIElement::UpdateChildren(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) {
    UIElement& last_x = *this;
    UIElement& last_y = *this;
    Dimension x = dx;
    Dimension y = dy;
    for(auto& child : m_Children) {
        if(child.m_Vertical) {
            y += last_y.Bottom();
            last_y = child;
            child.Update(ctx, mouse_down, mouse_x, mouse_y, dx, y);
        }
        else {
            x += last_x.Right();
            last_x = child;
            child.Update(ctx, mouse_down, mouse_x, mouse_y, x, dy);
        }
    }
}

UIResult Tickbox::Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) {
    UpdateChildren(ctx, mouse_down, mouse_x, mouse_y, dx, dy);

    Texture& tex = m_State ? m_TextureOn.get() : m_TextureOff.get();
    tex.Draw(ctx, dx + m_X, dy + m_Y, m_UnitSize, m_UnitSize);

	if(mouse_down && (IsPointInRect(mouse_x, mouse_y, dx + m_X, dy + m_Y, m_UnitSize, m_UnitSize))) {
		m_State = !m_State;
		return UIResult::Click;
	}
	return UIResult::None;
}

UIResult Button::Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) {
    UpdateChildren(ctx, mouse_down, mouse_x, mouse_y, dx, dy);

	m_Texture.get().Draw(ctx, dx + m_X, dy + m_Y, m_Width, m_Height, m_Rotation);

	if(IsPointInRect(mouse_x, mouse_y, dx + m_X, dy + m_Y, m_Width, m_Height)) {
		return mouse_down ? UIResult::Click : UIResult::Hover;
	}
	return UIResult::None;
}

ArrowSelect::ArrowSelect(Dimension x, Dimension y, Dimension unit_size, Span<const std::string> paths, Context& ctx, TextureLoader& loader) : UIElement(x, y), m_UnitSize(unit_size), m_Current(0), m_Left{ x, y, unit_size, unit_size, "LeftArrow.png", ctx, loader }, m_Right{ x + (2 * unit_size), y, unit_size, unit_size, "RightArrow.png", ctx, loader } {
    for(Dimension i = 0; i < paths.m_Size; ++i) {
        m_ItemTextures.emplace_back(loader.Get(paths.m_Data[i], ctx));
    }
}

UIResult ArrowSelect::Update(Context& ctx, bool mouse_down, Dimension mouse_x, Dimension mouse_y, Dimension dx, Dimension dy) {
    UpdateChildren(ctx, mouse_down, mouse_x, mouse_y, dx, dy);

    UIResult lres = m_Left.Update(ctx, mouse_down, mouse_x, mouse_y, dx, dy);
    if(lres == UIResult::Click) m_Current--;
    UIResult rres = m_Right.Update(ctx, mouse_down, mouse_x, mouse_y, dx, dy);
    if(rres == UIResult::Click) m_Current++;
    m_ItemTextures[m_Current % m_ItemTextures.size()].get().Draw(ctx, m_X + m_UnitSize, m_Y, m_UnitSize, m_UnitSize);
    return lres == UIResult::Click || rres == UIResult::Click ? UIResult::Click : UIResult::None;
}
