#pragma once

#include <Util.hpp>
#include <CWG.hpp>
#include <FX.hpp>
#include <Elements.hpp>

class Player {
public:
    static constexpr float MaxHealth = 1000.0f;

private:
    static constexpr float ProjectileSpeed = 10.0f;
    static constexpr Dimension MaxProjectiles = 10;

public:
    std::string m_Name;
    Piece m_Piece;
    Weapon m_Weapon;
    Color m_Color;

    Dimension m_Ammo;

    bool m_Dead{};

    Dimension m_X;
    Dimension m_Y;

    Dimension m_DamageBoost{};

    float m_Health{MaxHealth};

    bool m_AI{};
    std::array<Projectile, MaxProjectiles> m_Projectiles{};

public:
    Player(Piece piece, Weapon weapon, bool ai, Dimension x, Dimension y, Board& board, std::string  name, Color color);

    void Move(Board& board, Dimension dx, Dimension dy);
    std::vector<std::pair<Dimension, Dimension>> EnumerateValidPositions(Board& board) const;
    void PickupCheck(Board& board, Dimension x, Dimension y, Span<Pickup> pickups, SoundEffects& sound_effects);
    bool DoMoves(Context& ctx, Board& board, Span<Pickup> pickups, SoundEffects& sound_effects);
    bool DoWeapon(Context& ctx, WeaponTextures& textures, Span<Player> players);
    bool Hurt(float damage);
};
