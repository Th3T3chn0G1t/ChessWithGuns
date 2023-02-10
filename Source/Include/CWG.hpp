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

    AimTest,

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
    constexpr static Dimension SquareScale = 64;
    constexpr static Dimension Width = 6;
    constexpr static Dimension Height = 6;

private:

    std::array<Piece, Width * Height> m_Board{};

    std::unordered_map<Piece, std::reference_wrapper<Texture>> m_PieceTextures;

public:
    static bool IsInBounds(Dimension x, Dimension y);

    Board(TextureLoaderWrapper& loader, Context& ctx);

    void Draw(Context& ctx);

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
