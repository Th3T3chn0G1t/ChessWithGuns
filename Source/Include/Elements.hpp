// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

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
    Piece DoMove(Context& ctx, Board& board, Piece ignore, bool boosted, Dimension dx, Dimension dy);
};

class Pickup {
public:
    Dimension m_X;
    Dimension m_Y;

    Pickup(Board& board);
    void Place(Board& board);
};
