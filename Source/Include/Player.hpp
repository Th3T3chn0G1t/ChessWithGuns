// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <CWG.hpp>
#include <FX.hpp>
#include <Elements.hpp>

class Player {
public:
    static constexpr float MaxHealth = 100.0f;

private:
    static constexpr float ProjectileSpeed = 10.0f;
    static constexpr Dimension MaxProjectiles = 10;

public:
    std::string m_Name;
    Piece m_Piece;
    Weapon m_Weapon;
    Color m_Color;
    Color m_AmmoColor;

    Dimension m_Ammo;

    bool m_Dead{};

    Dimension m_X;
    Dimension m_Y;

    Dimension m_DamageBoost{};

    float m_Health{MaxHealth};

    bool m_AI{};
    std::array<Projectile, MaxProjectiles> m_Projectiles{};

public:
    Player(Piece piece, Weapon weapon, bool ai, Dimension x, Dimension y, Board& board, std::string  name, Color color, Color ammo_color);

    void Move(Board& board, Dimension dx, Dimension dy);
    std::vector<std::pair<Dimension, Dimension>> EnumerateValidPositions(Board& board) const;
    void PickupCheck(Board& board, Dimension x, Dimension y, Span<Pickup> pickups, SoundEffects& sound_effects);
    bool DoMoves(Context& ctx, Board& board, Span<Pickup> pickups, SoundEffects& sound_effects, Dimension dx, Dimension dy);
    bool DoWeapon(Context& ctx, WeaponTextures& textures, Span<Player> players, Dimension dx, Dimension dy);
    bool Hurt(float damage);
};
