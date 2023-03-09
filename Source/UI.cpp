#include <UI.hpp>

bool Tickbox::Update(Context& ctx) {
	if(m_TextureOn.get().m_Dummy) {
		ctx.DrawRect(m_X, m_Y, Board::SquareScale / 2, Board::SquareScale / 2, Color::Gray);
		Dimension ds = (Board::SquareScale / 2) - (Board::SquareScale / 3);
		ctx.DrawRect(m_X + (ds / 2), m_Y + (ds / 2), Board::SquareScale / 3, Board::SquareScale / 3, m_State ? Color::Green : Color::Red);
	}
	else {
		Texture& tex = m_State ? m_TextureOn.get() : m_TextureOff.get();
		tex.Draw(ctx, m_X, m_Y, Board::SquareScale / 2, Board::SquareScale / 2);
	}

	auto p = Context::GetMousePosition();
	if(ctx.IsMouseHeld() && (IsPointInRect(p.first, p.second, m_X, m_Y, Board::SquareScale / 2, Board::SquareScale / 2))) {
		m_State = !m_State;
		return true;
	}
	return false;
}

UIResult Button::Update(Context& ctx) {
	m_Texture.get().Draw(ctx, m_X, m_Y, m_Width, m_Height, m_Rotation);

	auto p = Context::GetMousePosition();
	if(IsPointInRect(p.first, p.second, m_X, m_Y, m_Width, m_Height)) {
		return ctx.IsMouseHeld() ? UIResult::Click : UIResult::Hover;
	}
	return UIResult::None;
}
