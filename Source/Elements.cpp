#include <Elements.hpp>
#include <Context.hpp>

Piece Projectile::DoMove(Context& ctx, Board& board, Piece ignore, bool boosted) {
    if(m_Shown) {
        float dx = m_Speed * cos(m_Rotation);
        float dy = m_Speed * sin(m_Rotation);
        m_X += dx;
        m_Y += dy;

        auto x = static_cast<Dimension>(m_X);
        auto y = static_cast<Dimension>(m_Y);

        ctx.DrawRect(x, y, ProjectileScale, ProjectileScale, boosted ? Color::Blue : Color::Red);

        if(!Board::IsInBounds(x / Board::SquareScale, y / Board::SquareScale)) {
            m_Shown = false;
            return Piece::None;
        }

        Piece piece = board.Get(x / Board::SquareScale, y / Board::SquareScale);
        if(piece != Piece::None && piece != ignore && !IsPickup(piece)) {
            m_Shown = false;
            return piece;
        }
    }

    return Piece::None;
}

Pickup::Pickup(Board& board) {
    do {
        m_X = Context::UnsignedRandRange(Board::Width - 1);
        m_Y = Context::UnsignedRandRange(Board::Height - 1);
    } while(board.Get(m_X, m_Y) != Piece::None);

    if(Context::UnsignedRandRange(3)) board.Set(m_X, m_Y, Piece::AmmoPickup);
    else if(Context::UnsignedRandRange(2)) board.Set(m_X, m_Y, Piece::BoostPickup);
    else board.Set(m_X, m_Y, Piece::HealthPickup);
}

void Pickup::Place(Board& board) {
    Dimension x = m_X;
    Dimension y = m_Y;

    do {
        m_X = Context::UnsignedRandRange(Board::Width - 1);
        m_Y = Context::UnsignedRandRange(Board::Height - 1);
    } while(board.Get(m_X, m_Y) != Piece::None);

    if(Context::UnsignedRandRange(3)) board.Set(m_X, m_Y, Piece::AmmoPickup);
    else if(Context::UnsignedRandRange(2)) board.Set(m_X, m_Y, Piece::BoostPickup);
    else board.Set(m_X, m_Y, Piece::HealthPickup);

    board.Set(x, y, Piece::None);
}
