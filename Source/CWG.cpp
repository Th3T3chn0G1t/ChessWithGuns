#include <CWG.hpp>
#include "Texture.hpp"

std::vector<PieceMove> EnumeratePieceMoves(Piece piece) {
    switch(piece) {
        case Piece::AmmoPickup:
        case Piece::HealthPickup:
        case Piece::BoostPickup:
        case Piece::None: return {};

        case Piece::WhitePawn: return {{0, -1}};
        case Piece::BlackPawn: return {{0, 1}};

        case Piece::WhiteRook:
        case Piece::BlackRook: return {{0, DimensionMax, true}, {0, DimensionMin, true}, {DimensionMax, 0, true}, {DimensionMin, 0, true}};

        case Piece::WhiteBishop:
        case Piece::BlackBishop: return {{DimensionMax, DimensionMax, true}, {DimensionMax, DimensionMin, true}, {DimensionMin, DimensionMax, true}, {DimensionMin, DimensionMin, true}};

        case Piece::WhiteKnight:
        case Piece::BlackKnight: return {{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {-2, 1}, {2, -1}, {-2, -1}};

        case Piece::WhiteKing:
        case Piece::BlackKing: return {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};

        case Piece::WhiteQueen:
        case Piece::BlackQueen: return {{0, DimensionMax, true}, {DimensionMax, DimensionMax, true}, {DimensionMax, 0, true}, {DimensionMax, DimensionMin, true}, {0, DimensionMin, true}, {DimensionMin, DimensionMin, true}, {DimensionMin, 0, true}, {DimensionMin, DimensionMax, true}};
    }
}

bool IsPickup(Piece piece) {
    return piece == Piece::AmmoPickup || piece == Piece::HealthPickup || piece == Piece::BoostPickup;
}

std::unordered_map<Weapon, float> WeaponStats::WeaponDamages {
    {Weapon::None, 0.0f},
    {Weapon::AimTest, 11.0f},
    {Weapon::Pistol, 9.0f},
    {Weapon::Shotgun, 8.0f},
    {Weapon::ScienceGun, 5.0f},
    {Weapon::Rifle, 11.0f},
    {Weapon::RocketLauncher, 37.0f}
};

std::unordered_map<Weapon, float> WeaponStats::WeaponSpreads {
    {Weapon::None, 0.0f},
    {Weapon::AimTest, ((15.0f * static_cast<float>(M_PI)) / 180.0f)},
    {Weapon::Pistol, ((10.0f * static_cast<float>(M_PI)) / 180.0f)},
    {Weapon::Shotgun, ((35.0f * static_cast<float>(M_PI)) / 180.0f)},
    {Weapon::ScienceGun, ((15.0f * static_cast<float>(M_PI)) / 180.0f)},
    {Weapon::Rifle, ((5.0f * static_cast<float>(M_PI)) / 180.0f)},
    {Weapon::RocketLauncher, ((35.0f * static_cast<float>(M_PI)) / 180.0f)}
};

std::unordered_map<Weapon, float> WeaponStats::WeaponVariances {
    {Weapon::None, 0.0f},
    {Weapon::AimTest, 4.0f},
    {Weapon::Pistol, 2.0f},
    {Weapon::Shotgun, 1.0f},
    {Weapon::ScienceGun, 3.0f},
    {Weapon::Rifle, 4.0f},
    {Weapon::RocketLauncher, 10.0f}
};

std::unordered_map<Weapon, Dimension> WeaponStats::WeaponCounts {
    {Weapon::None, 0},
    {Weapon::AimTest, 300},
    {Weapon::Pistol, 1},
    {Weapon::Shotgun, 7},
    {Weapon::ScienceGun, 3},
    {Weapon::Rifle, 2},
    {Weapon::RocketLauncher, 1}
};

std::unordered_map<Weapon, Dimension> WeaponStats::WeaponAmmos {
    {Weapon::None, 0},
    {Weapon::AimTest, 6},
    {Weapon::Pistol, 20},
    {Weapon::Shotgun, 15},
    {Weapon::ScienceGun, 6},
    {Weapon::Rifle, 10},
    {Weapon::RocketLauncher, 1}
};

WeaponTextures::WeaponTextures(TextureLoaderWrapper& loader, Context& ctx) {
    m_Textures.insert({Weapon::None, Texture::Dummy});

    m_Textures.insert({Weapon::AimTest, loader.Get("AimTest.png", ctx)});
    m_Textures.insert({Weapon::Pistol, loader.Get("Pistol.png", ctx)});
    m_Textures.insert({Weapon::Shotgun, loader.Get("Shotgun.png", ctx)});
    m_Textures.insert({Weapon::ScienceGun, loader.Get("ScienceGun.png", ctx)});
    m_Textures.insert({Weapon::Rifle, loader.Get("Rifle.png", ctx)});
    m_Textures.insert({Weapon::RocketLauncher, loader.Get("RocketLauncher.png", ctx)});
}

SoundEffects::SoundEffects(SoundEffectLoader& loader) {
    m_WeaponSounds.insert({Weapon::AimTest, loader.Get("Explosion.wav")});
    m_WeaponSounds.insert({Weapon::Pistol, loader.Get("Explosion.wav")});
    m_WeaponSounds.insert({Weapon::Shotgun, loader.Get("Explosion.wav")});
    m_WeaponSounds.insert({Weapon::ScienceGun, loader.Get("Explosion.wav")});
    m_WeaponSounds.insert({Weapon::Rifle, loader.Get("Explosion.wav")});
    m_WeaponSounds.insert({Weapon::RocketLauncher, loader.Get("Explosion.wav")});

    m_PieceSounds.insert({Piece::AmmoPickup, loader.Get("Power.wav")});
    m_PieceSounds.insert({Piece::HealthPickup, loader.Get("Power.wav")});
    m_PieceSounds.insert({Piece::BoostPickup, loader.Get("Power.wav")});
}
