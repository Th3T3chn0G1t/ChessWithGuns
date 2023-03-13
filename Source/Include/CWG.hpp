// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#pragma once

#include <Util.hpp>
#include <SoundEffect.hpp>

enum class Piece {
    None,

    WhitePawn,
    WhiteRook,
    WhiteBishop,
    WhiteKnight,
    WhiteKing,
    WhiteQueen,

    BlackPawn,
    BlackRook,
    BlackBishop,
    BlackKnight,
    BlackKing,
    BlackQueen,

    AmmoPickup,
    HealthPickup,
    BoostPickup
};

enum class Weapon {
    None,

    Grenade,
    Pistol,
    Shotgun,
    ScienceGun,
    Rifle,
    RocketLauncher
};

struct PieceMove {
    Dimension m_Dx;
    Dimension m_Dy;
    bool m_Fill;
};

class Context;
class Texture;
struct TextureLoaderWrapper;

class Board {
public:
    static Dimension SquareScale;
    static Dimension Width;
    static Dimension Height;

    std::unordered_map<Piece, std::reference_wrapper<Texture>> m_PieceTextures;
private:
    std::vector<Piece> m_Board;

public:
    static bool IsInBounds(Dimension x, Dimension y);

    Board(TextureLoaderWrapper& loader, Context& ctx);

    void Draw(Context& ctx, Dimension x, Dimension y);

    void Set(Dimension x, Dimension y, Piece piece);
    Piece Get(Dimension x, Dimension y);
};

std::vector<PieceMove> EnumeratePieceMoves(Piece piece);
bool IsPickup(Piece piece);

struct WeaponStats {
    static std::unordered_map<Weapon, float> WeaponDamages;
    static std::unordered_map<Weapon, float> WeaponSpreads;
    static std::unordered_map<Weapon, float> WeaponVariances;
    static std::unordered_map<Weapon, Dimension> WeaponCounts;
    static std::unordered_map<Weapon, Dimension> WeaponAmmos;
};

class WeaponTextures {
public:
    std::unordered_map<Weapon, std::reference_wrapper<Texture>> m_Textures;

    WeaponTextures(TextureLoaderWrapper& loader, Context& ctx);
};

class SoundEffects {
public:
    std::unordered_map<Weapon, std::reference_wrapper<SoundEffect>> m_WeaponSounds;
    std::unordered_map<Piece, std::reference_wrapper<SoundEffect>> m_PieceSounds;

    SoundEffects(SoundEffectLoader& loader);
};
