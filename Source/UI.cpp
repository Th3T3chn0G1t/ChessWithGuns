#include <UI.hpp>

bool Tickbox::Update(Context& ctx, bool mouse_down) {
    Texture& tex = m_State ? m_TextureOn.get() : m_TextureOff.get();
    tex.Draw(ctx, m_X, m_Y, m_UnitSize, m_UnitSize);

	auto p = Context::GetMousePosition();
	if(mouse_down && (IsPointInRect(p.first, p.second, m_X, m_Y, m_UnitSize, m_UnitSize))) {
		m_State = !m_State;
		return true;
	}
	return false;
}

UIResult Button::Update(Context& ctx, bool mouse_down) {
	m_Texture.get().Draw(ctx, m_X, m_Y, m_Width, m_Height, m_Rotation);

	auto p = Context::GetMousePosition();
	if(IsPointInRect(p.first, p.second, m_X, m_Y, m_Width, m_Height)) {
		return mouse_down ? UIResult::Click : UIResult::Hover;
	}
	return UIResult::None;
}

ArrowSelect::ArrowSelect(Dimension x, Dimension y, Dimension unit_size, Span<std::string> paths, Context& ctx, TextureLoader& loader) : m_X(x), m_Y(y), m_UnitSize(unit_size), m_Current(0), m_Left{ x, y, unit_size, unit_size, "LeftArrow.png", ctx, loader }, m_Right{ x + (2 * unit_size), y, unit_size, unit_size, "RightArrow.png", ctx, loader } {
    for(Dimension i = 0; i < paths.m_Size; ++i) {
        m_ItemTextures.emplace_back(loader.Get(paths.m_Data[i], ctx));
    }
}

bool ArrowSelect::Update(Context& ctx, bool mouse_down) {
    UIResult lres = m_Left.Update(ctx, mouse_down);
    if(lres == UIResult::Click) m_Current--;
    UIResult rres = m_Right.Update(ctx, mouse_down);
    if(rres == UIResult::Click) m_Current++;
    m_ItemTextures[m_Current % m_ItemTextures.size()].get().Draw(ctx, m_X + m_UnitSize, m_Y, m_UnitSize, m_UnitSize);
    return lres == UIResult::Click || rres == UIResult::Click;
}
