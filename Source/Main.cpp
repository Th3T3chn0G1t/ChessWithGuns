// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Util.hpp>
#include <FX.hpp>
#include <CWG.hpp>
#include <Context.hpp>
#include <Texture.hpp>
#include <Elements.hpp>
#include <Player.hpp>
#include <SoundEffect.hpp>

int main() {
    restart:;

    GameSettings settings{};

    Context::Width = 640;
    Context::Height = 480;

    Context ctx{};
    TextureLoaderWrapper loader(TextureLoader(ctx.m_ResourcePath));
    SoundEffectLoader sfx_loader(ctx.m_ResourcePath);
    WeaponTextures weapon_textures(loader, ctx);
    SoundEffects sound_effects(sfx_loader);

    settings.m_UISettings.m_TitleScrollers = 15;
    DoMenu(ctx, settings, loader, sfx_loader);

    SoundEffect& next_turn = sfx_loader.Get("Turn.wav");
	Context::StopSounds();
    next_turn.Play();

    Board::Width = 8;
    Board::Height = 8;

    Board board(loader, ctx);

    std::array<Player, 2> players {
        Player{
            settings.m_WhitePiece, settings.m_WhiteWeapon, settings.m_WhiteAI,
            Board::Width - 1, Board::Height - 1, board,
            "White", Color::White, Color::Black
        },
        Player{
            settings.m_BlackPiece, settings.m_BlackWeapon, settings.m_BlackAI,
            0, 0, board,
            "Black", Color::Black, Color::White
        }
    };

    std::array<Pickup, 2> pickups{
        Pickup{board},
        Pickup{board}
    };

    Dimension turn = 0;
    Dimension dead = 0;

    Dimension frames_per_turn = settings.m_MoveTimer ? 45 : 0;
    Dimension frames_this_turn = 0;
    bool moved = false;

	SoundEffect& game_song = sfx_loader.Get("PawnWithAShotgun.wav");
	game_song.Loop(-1);
	while(ctx.Update()) {
        ctx.Clear(Color::DarkGray);

        auto& player = players[turn];
        frames_this_turn++;
        if(player.m_Dead) {
            if(++turn >= players.size()) turn = 0;
            continue;
        }

        {
            Dimension cx = (Board::Width / 2) * Board::SquareScale;
            Dimension cy = (Board::Height / 2) * Board::SquareScale;

            Dimension pcx = player.m_X * Board::SquareScale;
            Dimension pcy = player.m_Y * Board::SquareScale;

            Dimension bx = cx - pcx;
            Dimension by = cy - pcy;
            board.Draw(ctx, bx, by);

            bool did_move = false;
            bool did_weapon = false;
            if(!moved) {
                did_move = player.DoMoves(ctx, board, Span<Pickup>(pickups), sound_effects, bx, by);
                if(!did_move) did_weapon = player.DoWeapon(ctx, weapon_textures, Span<Player>(players), bx, by);
                if(settings.m_SFX && did_move) next_turn.Play();
                else if(settings.m_SFX && did_weapon) sound_effects.m_WeaponSounds.at(player.m_Weapon).get().Play();
                moved = did_move || did_weapon;
            }

            Dimension health_width = 240;
            Dimension health_height = 32;
            Dimension ammo_padding = 4;
            Dimension health_border = 2;

            Dimension health_x = Context::Width - health_width;
            float health_portion = static_cast<float>(player.m_Health) / static_cast<float>(Player::MaxHealth);
            auto health_current = static_cast<Dimension>(static_cast<float>(health_width) * health_portion);
            ctx.DrawRect(health_x, 0, health_current, health_height, player.m_Color);
            ctx.DrawRect(health_x + health_current, 0, health_width - health_current, health_height, Color::Gray);

            ctx.DrawRect(health_x, 0, health_width, health_border, Color::Red);
            ctx.DrawRect(health_x, health_height - health_border, health_width, health_border, Color::Red);
            ctx.DrawRect(health_x, 0, health_border, health_height, Color::Red);
            ctx.DrawRect(health_x + health_width - health_border, 0, health_border, health_height, Color::Red);

            for(Dimension j = 0; j < player.m_Ammo; ++j) {
                ctx.DrawRect(health_x + (2 * ammo_padding * j) + ammo_padding, ammo_padding, ammo_padding, health_height - (2 * ammo_padding), player.m_AmmoColor);
            }

            if((frames_this_turn >= frames_per_turn) && moved) {
                moved = false;
                frames_this_turn = 0;
                if(++turn >= players.size()) turn = 0;
            }

            for(auto& fired : players) {
                for(Projectile& projectile : fired.m_Projectiles) {
                    Piece hit = projectile.DoMove(ctx, board, fired.m_Piece, !!fired.m_DamageBoost, bx, by);
                    if(hit != Piece::None) {
                        projectile.m_Shown = false;
                        float damage = WeaponStats::WeaponDamages[fired.m_Weapon] + Context::SignedRandRange(WeaponStats::WeaponVariances[fired.m_Weapon]) + static_cast<float>(fired.m_DamageBoost);
                        ctx.m_ShakeIntensity = static_cast<Dimension>(damage);
                        for(auto& other : players) {
                            if(hit == other.m_Piece) {
                                bool death = other.Hurt(damage);
                                if(death) {
                                    other.m_Dead = true;
                                    board.Set(other.m_X, other.m_Y, Piece::None);
                                    dead++;
                                    if(dead >= players.size() - 1) {
                                        Context::Dialog("Game Over", fired.m_Name + " won!");
                                        goto restart;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
