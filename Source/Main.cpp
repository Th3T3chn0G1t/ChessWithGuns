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
#include <UI.hpp>

int main() {
    restart:;
    Context ctx{};
    TextureLoaderWrapper loader(TextureLoader("Resources"));
    SoundEffectLoader sfx_loader("Resources");
    WeaponTextures weapon_textures(loader, ctx);
    SoundEffects sound_effects(sfx_loader);

    Board::Width = (Context::Width / Board::SquareScale) + 3;
    Board::Height = (Context::Height / Board::SquareScale) + 3;
    Board menu_board(loader, ctx);
    Texture& title = loader.Get("Title.png", ctx);
    Dimension x_off = 0;
    Dimension y_off = 0;
    float r = 0;
    SoundEffect& title_song = sfx_loader.Get("Title.wav");
    SoundEffect& next_turn = sfx_loader.Get("Turn.wav");

    struct MenuScroller {
        Dimension m_X;
        Dimension m_Y;
        Piece m_Piece;

        MenuScroller() {
            if(Context::UnsignedRandRange(2)) {
                m_X = Board::Width - 1;
                m_Y = Context::UnsignedRandRange(Board::Height);
            }
            else {
                m_X = Context::UnsignedRandRange(Board::Width);
                m_Y = Board::Height - 1;
            }
            m_Piece = static_cast<Piece>(Context::UnsignedRandRange(15));
        }

        void Tick(Dimension x, Dimension y, Board& board) {
            board.Set(m_X, m_Y, Piece::None);
            m_X += x;
            m_Y += y;
            if(m_X < 0 || m_Y < 0) *this = MenuScroller();
            else board.Set(m_X, m_Y, m_Piece);
        }
    };
    std::vector<MenuScroller> scrollers;
    scrollers.resize(15);

    for(auto& scroller : scrollers) {
        scroller.m_X = Context::UnsignedRandRange(Board::Width - 1);
        scroller.m_Y = Context::UnsignedRandRange(Board::Height - 1);
        menu_board.Set(scroller.m_X, scroller.m_Y, scroller.m_Piece);
    }

	Tickbox sfx { 0, 0, Board::SquareScale / 2, true, "SFX_On.png", "SFX_Off.png", ctx, loader.m_Loader };
    Dimension play_y = (Board::SquareScale / 2) + (Context::Width / 4);
	Button play { (Context::Width / 2) - (Context::Width / 12), Context::Height - (Board::SquareScale + (Board::SquareScale / 2)), Context::Width / 6, Context::Width / 18, "PlayButton.png", ctx, loader.m_Loader };

    std::array<std::string, 7> weapon_paths {
        "None.png",
        "AimTest.png",
        "Pistol.png",
        "Shotgun.png",
        "ScienceGun.png",
        "Rifle.png",
        "RocketLauncher.png"
    };

    std::array<std::string, 6> black_piece_paths {
        "BlackPawn.png",
        "BlackRook.png",
        "BlackBishop.png",
        "BlackKnight.png",
        "BlackKing.png",
        "BlackQueen.png"
    };
    ArrowSelect black_piece_select { Board::SquareScale, play_y + (Board::SquareScale / 2), Board::SquareScale, Span<std::string>(black_piece_paths), ctx, loader.m_Loader };
    ArrowSelect black_weapon_select { Board::SquareScale, black_piece_select.m_Y + Board::SquareScale + (Board::SquareScale / 4), Board::SquareScale, Span<std::string>(weapon_paths), ctx, loader.m_Loader };

    std::array<std::string, 6> white_piece_paths {
        "WhitePawn.png",
        "WhiteRook.png",
        "WhiteBishop.png",
        "WhiteKnight.png",
        "WhiteKing.png",
        "WhiteQueen.png"
    };
    ArrowSelect white_piece_select { Context::Width - (4 * Board::SquareScale), play_y + (Board::SquareScale / 2), Board::SquareScale, Span<std::string>(white_piece_paths), ctx, loader.m_Loader };
    ArrowSelect white_weapon_select { white_piece_select.m_X, white_piece_select.m_Y + Board::SquareScale + (Board::SquareScale / 4), Board::SquareScale, Span<std::string>(weapon_paths), ctx, loader.m_Loader };

    Tickbox black_human { black_piece_select.m_X + Board::SquareScale, black_piece_select.m_Y + (2 * Board::SquareScale) + (Board::SquareScale / 2), Board::SquareScale, true, "Person.png", "Computer.png", ctx, loader.m_Loader };
    Tickbox white_human { white_piece_select.m_X + Board::SquareScale, white_piece_select.m_Y + (2 * Board::SquareScale) + (Board::SquareScale / 2), Board::SquareScale, true, "Person.png", "Computer.png", ctx, loader.m_Loader };

	title_song.Loop(-1);
    while(true) {
		if(!ctx.Update()) return 0;

        ctx.Clear(Color::Gray);

        {
            menu_board.Draw(ctx, x_off--, y_off--);
            if(x_off <= -Board::SquareScale) {
                x_off = 0;
                for(auto& scroller : scrollers) scroller.Tick(-1, 0, menu_board);
            }
            if(y_off <= -Board::SquareScale) {
                y_off = 0;
                for(auto& scroller : scrollers) scroller.Tick(0, -1, menu_board);
            }
        }

		float rot = 2 * sinf(r);
        {
            title.Draw(ctx, (Context::Width / 2) - (Context::Width / 2), Board::SquareScale / 2, Context::Width, Context::Width / 4, rot);
            r += 0.05f;
            if(r >= M_PI * 2) r = 0;
        }

		bool pressed = ctx.WasMousePressed();
		if(sfx.Update(ctx, pressed)) {
			if(sfx.m_State) {
                next_turn.Play();
                title_song.Loop(-1);
            }
			else Context::StopSounds();
		}

        if(black_piece_select.Update(ctx, pressed) && sfx.m_State) next_turn.Play();
        if(black_weapon_select.Update(ctx, pressed) && sfx.m_State) next_turn.Play();
        if(black_human.Update(ctx, pressed) && sfx.m_State) next_turn.Play();

        if(white_piece_select.Update(ctx, pressed) && sfx.m_State) next_turn.Play();
        if(white_weapon_select.Update(ctx, pressed) && sfx.m_State) next_turn.Play();
        if(white_human.Update(ctx, pressed) && sfx.m_State) next_turn.Play();

		switch(play.Update(ctx, pressed)) {
			case UIResult::None: {
				play.m_Rotation = 0;
				break;
			}
			case UIResult::Hover: {
				play.m_Rotation = 2 * rot;
				break;
			}
			case UIResult::Click: goto game;
		}
    }

	game: Context::StopSounds();
    next_turn.Play();

	bool do_sfx = sfx.m_State;

    Board::Width = 8;
    Board::Height = 8;
    ctx.Resize(Board::Width * Board::SquareScale + Context::SidebarWidth, Board::Height * Board::SquareScale);

    Board board(loader, ctx);

    std::array<Player, 2> players {
        Player{
            static_cast<Piece>(static_cast<Dimension>(Piece::WhitePawn) + (white_piece_select.m_Current % white_piece_paths.size())),
            static_cast<Weapon>(white_weapon_select.m_Current % weapon_paths.size()),
            !white_human.m_State,
            Board::Width - 1, Board::Height - 1, board,
            "White", Color::White
        },
        Player{
            static_cast<Piece>(static_cast<Dimension>(Piece::BlackPawn) + (black_piece_select.m_Current % black_piece_paths.size())),
            static_cast<Weapon>(black_weapon_select.m_Current % weapon_paths.size()),
            !black_human.m_State,
            0, 0, board,
            "Black", Color::Black
        }
    };

    std::array<Pickup, 2> pickups{
        Pickup{board},
        Pickup{board}
    };

    Dimension turn = 0;
    Dimension dead = 0;

    while(ctx.Update()) {
        ctx.Clear(Color::Gray);
        board.Draw(ctx, 0, 0);

        auto& player = players[turn];
        if(player.m_Dead) {
            if(++turn >= players.size()) turn = 0;
            goto ui;
        }

        {
            bool did_move = player.DoMoves(ctx, board, Span<Pickup>(pickups), sound_effects);
            bool did_weapon = player.DoWeapon(ctx, weapon_textures, Span<Player>(players));
            if(do_sfx && did_move) next_turn.Play();
            else if(do_sfx && did_weapon) sound_effects.m_WeaponSounds.at(player.m_Weapon).get().Play();

            if(did_move || did_weapon) {
                if(++turn >= players.size()) turn = 0;
            }
        }

        for(auto& fired : players) {
            for(Projectile& projectile : fired.m_Projectiles) {
                Piece hit = projectile.DoMove(ctx, board, fired.m_Piece, !!fired.m_DamageBoost);
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
                            goto ui;
                        }
                    }
                }
            }
        }

        ui: {
            for(Dimension i = 0; i < players.size(); ++i) {
                if(players[i].m_Dead) continue;

                float health_portion = static_cast<float>(players[i].m_Health) / static_cast<float>(Player::MaxHealth);
                ctx.DrawRect(Context::Width - Context::SidebarWidth, i * 32, static_cast<Dimension>(static_cast<float>(Context::SidebarWidth) * health_portion), 32, players[i].m_Color);

                for(Dimension j = 0; j < players[i].m_Ammo; ++j) {
                    ctx.DrawRect(Context::Width - Context::SidebarWidth + 4 * j + 1, Context::Height - (i + 1) * 32 + 1, 2, 2, players[i].m_Color);
                }
            }
            ctx.DrawRect(Context::Width - Context::SidebarWidth, 0, 2, Context::Height, Color::Red);
        }
    }
}
