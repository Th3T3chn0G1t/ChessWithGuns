// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Player.hpp>
#include <Context.hpp>
#include <Texture.hpp>

Player::Player(Piece piece, Weapon weapon, bool ai, Dimension x, Dimension y, Board& board, std::string  name, Color color) : m_X(0), m_Y(0), m_Piece(piece), m_Weapon(weapon), m_AI(ai), m_Name(std::move(name)), m_Color(color) {
    board.Set(m_X, m_Y, Piece::None);
    Move(board, x, y);
    m_Ammo = WeaponStats::WeaponAmmos[weapon];
};

void Player::Move(Board& board, Dimension dx, Dimension dy) {
    board.Set(m_X, m_Y, Piece::None);

    m_X += dx;
    m_Y += dy;
    if(!Board::IsInBounds(m_X, m_Y)) throw std::runtime_error("Attempt to move player out of bounds");

    board.Set(m_X, m_Y, m_Piece);
}

std::vector<std::pair<Dimension, Dimension>> Player::EnumerateValidPositions(Board& board) const {
    auto piece_moves = EnumeratePieceMoves(m_Piece);

    std::vector<std::pair<Dimension, Dimension>> positions{};

    for(PieceMove& move : piece_moves) {
        if(!move.m_Fill) {
            Piece at = board.Get(m_X + move.m_Dx, m_Y + move.m_Dy);
            if(at == Piece::None || IsPickup(at)) {
                positions.emplace_back(move.m_Dx, move.m_Dy);
            }
        }
        else {
            Dimension dx = 0;
            Dimension dy = 0;
            while(true) {
                if(move.m_Dx > 0 && dx < move.m_Dx) ++dx;
                else if(move.m_Dx < 0 && dx > move.m_Dx) --dx;
                else if(move.m_Dx) break;

                if(move.m_Dy > 0 && dy < move.m_Dy) ++dy;
                else if(move.m_Dy < 0 && dy > move.m_Dy) --dy;
                else if(move.m_Dy) break;

                if(!Board::IsInBounds(m_X + dx, m_Y + dy)) break;

                Piece at = board.Get(m_X + dx, m_Y + dy);
                if(IsPickup(at)) {
                    positions.emplace_back(dx, dy);
                    break;
                }
                else if(at != Piece::None) break;

                positions.emplace_back(dx, dy);
            }
        }
    }

    return positions;
}

void Player::PickupCheck(Board& board, Dimension x, Dimension y, Span<Pickup> pickups, SoundEffects& sound_effects) {
    Piece at = board.Get(x, y);
    if(at == Piece::AmmoPickup) {
        m_Ammo += 5;
        if(m_Ammo > WeaponStats::WeaponAmmos[m_Weapon]) m_Ammo = WeaponStats::WeaponAmmos[m_Weapon];
    }
    else if(at == Piece::HealthPickup) {
        m_Health += 7;
        if(m_Health > MaxHealth) m_Health = MaxHealth;
    }
    else if(at == Piece::BoostPickup) {
        m_DamageBoost = 5;
    }

    if(IsPickup(at)) {
        sound_effects.m_PieceSounds.at(at).get().Play();
        for(size_t i = 0; i < pickups.m_Size; ++i) {
            if(pickups.m_Data[i].m_X == x && pickups.m_Data[i].m_Y == y) {
                pickups.m_Data[i].Place(board);
                return;
            }
        }
        Context::Dialog("Error", "Invalid Pickup at " + std::to_string(x) + " " + std::to_string(y));
    }
}

bool Player::DoMoves(Context& ctx, Board& board, Span<Pickup> pickups, SoundEffects& sound_effects) {
    auto positions = EnumerateValidPositions(board);
    if(!m_AI) {
        for(auto& position : positions) {
            Dimension new_x = m_X + position.first;
            Dimension new_y = m_Y + position.second;

            if(!Board::IsInBounds(new_x, new_y)) continue;

            ctx.DrawRect(new_x * Board::SquareScale, new_y * Board::SquareScale, Board::SquareScale / 2, Board::SquareScale / 2, Color::Green);

            auto pos = Context::GetMousePosition();

            if(IsPointInRect(pos.first, pos.second, new_x * Board::SquareScale, new_y * Board::SquareScale, Board::SquareScale, Board::SquareScale)) {
                if(ctx.WasMousePressed()) {
                    PickupCheck(board, new_x, new_y, pickups, sound_effects);

                    Move(board, position.first, position.second);
                    return true;
                }
            }
        }
    }

    if(m_AI && Context::UnsignedRandRange(2)) {
        if(positions.empty()) return false;

        for(auto& position : positions) {
            if(!Board::IsInBounds(m_X + position.first, m_Y + position.second)) continue;

            Piece at = board.Get(m_X + position.first, m_Y + position.second);
            if(IsPickup(at)) {
                PickupCheck(board, m_X + position.first, m_Y + position.second, pickups, sound_effects);

                Move(board, position.first, position.second);
                return true;
            }
        }

        auto& position = positions[Context::UnsignedRandRange((int) positions.size())];
        if(!Board::IsInBounds(m_X + position.first, m_Y + position.second)) return false;

        PickupCheck(board, m_X + position.first, m_Y + position.second, pickups, sound_effects);

        Move(board, position.first, position.second);
        return true;
    }

    return false;
}

bool Player::DoWeapon(Context& ctx, WeaponTextures& textures, Span<Player> players) {
    auto pos = Context::GetMousePosition();
    float rot = atan(static_cast<float>(pos.second - m_Y * Board::SquareScale) / static_cast<float>(pos.first - m_X * Board::SquareScale));
    textures.m_Textures.at(m_Weapon).get().Draw(ctx, m_X * Board::SquareScale, m_Y * Board::SquareScale, Board::SquareScale, Board::SquareScale, (rot * 180.0f) / static_cast<float>(M_PI));

    if(m_Ammo <= 0) return false;

    if(!m_AI) {
        if(ctx.WasMousePressed()) {
            m_Ammo--;
            if(m_DamageBoost) m_DamageBoost -= Context::UnsignedRandRange(2);
            if(m_DamageBoost < 0) m_DamageBoost = 0;
            rot += pos.first - m_X * Board::SquareScale < 0 ? M_PI : 0;
            for(Dimension i = 0; i < WeaponStats::WeaponCounts[m_Weapon]; ++i) {
                for(Projectile& projectile : m_Projectiles) {
                    if(!projectile.m_Shown) {
                        projectile = Projectile{static_cast<float>(m_X * Board::SquareScale), static_cast<float>(m_Y * Board::SquareScale), rot + Context::SignedRandRange(WeaponStats::WeaponSpreads[m_Weapon]), ProjectileSpeed, true};
                        break;
                    }
                }
            }
            return true;
        }
    }
    else if(Context::UnsignedRandRange(2)) {
        m_Ammo--;
        if(m_DamageBoost) m_DamageBoost -= Context::UnsignedRandRange(2);
        if(m_DamageBoost < 0) m_DamageBoost = 0;
        Player& other = players.m_Data[Context::UnsignedRandRange(static_cast<Dimension>(players.m_Size))];
        Dimension dx = other.m_X - m_X;
        rot = atan(static_cast<float>(other.m_Y - m_Y) / static_cast<float>(dx));
        rot += dx < 0 ? M_PI : 0;
        for(Dimension i = 0; i < WeaponStats::WeaponCounts[m_Weapon]; ++i) {
            for(Projectile& projectile : m_Projectiles) {
                if(!projectile.m_Shown) {
                    projectile = Projectile{static_cast<float>(m_X * Board::SquareScale), static_cast<float>(m_Y * Board::SquareScale), rot + Context::SignedRandRange(WeaponStats::WeaponSpreads[m_Weapon]), ProjectileSpeed, true};
                    break;
                }
            }
        }
        return true;
    }

    return false;
}

bool Player::Hurt(float damage) {
    m_Health -= damage;
    return m_Health <= 0.0f;
}
