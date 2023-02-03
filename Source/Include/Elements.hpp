#pragma once

#include <Util.hpp>
#include <CWG.hpp>

class Projectile {
private:
    static constexpr Dimension ProjectileScale = 4;

public:
    float m_X;
    float m_Y;
    float m_Rotation;
    float m_Speed;
    bool m_Shown;

public:
    Piece DoMove(Context& ctx, Board& board, Piece ignore, bool boosted);
};
