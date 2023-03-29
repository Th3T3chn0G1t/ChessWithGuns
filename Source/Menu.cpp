// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <CWG.hpp>
#include <Context.hpp>
#include <Texture.hpp>
#include <UI.hpp>

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

static const std::array<std::string, 7> weapon_paths {
    "None.png",
    "Grenade.png",
    "Pistol.png",
    "Shotgun.png",
    "ScienceGun.png",
    "Rifle.png",
    "RocketLauncher.png"
};

static const std::array<std::string, 6> black_piece_paths {
    "BlackPawn.png",
    "BlackRook.png",
    "BlackBishop.png",
    "BlackKnight.png",
    "BlackKing.png",
    "BlackQueen.png"
};

static const std::array<std::string, 6> white_piece_paths {
    "WhitePawn.png",
    "WhiteRook.png",
    "WhiteBishop.png",
    "WhiteKnight.png",
    "WhiteKing.png",
    "WhiteQueen.png"
};

void DoMenu(Context& ctx, GameSettings& settings, TextureLoaderWrapper& loader, SoundEffectLoader& sfx_loader) {




    Dimension title_width = Context::Width;
    Dimension title_height = title_width / 4;
    Dimension title_x = Centre(Context::Width, title_width);
    Dimension title_y = Board::SquareScale / 2;

    Dimension tickbox_scale = Board::SquareScale / 2;
    Dimension tickbox_x = 0;
    Dimension tickbox_y = 0;

    Dimension play_button_width = Context::Width / 6;
    Dimension play_button_height = play_button_width / 3;
    Dimension play_button_x = Centre(Context::Width, play_button_width);
    Dimension play_button_y = Context::Height - (Board::SquareScale + (Board::SquareScale / 2));

    Dimension player_select_scale = Board::SquareScale;
    Dimension player_select_inset = Board::SquareScale;
    Dimension player_select_item_offset = Board::SquareScale / 4;
    Dimension player_select_y = title_y + title_height + (Board::SquareScale / 2);

    Dimension ai_tickbox_scale = Board::SquareScale;
    Dimension ai_tickbox_offset = Board::SquareScale / 2;





    std::vector<MenuScroller> scrollers;
    scrollers.resize(settings.m_UISettings.m_TitleScrollers);

    Board::Width = (Context::Width / Board::SquareScale) + 3;
    Board::Height = (Context::Height / Board::SquareScale) + 3;
    Board menu_board(loader, ctx);

    Texture& title = loader.Get("Title.png", ctx);
    SoundEffect& title_song = sfx_loader.Get("Title.wav");
    SoundEffect& next_turn = sfx_loader.Get("Turn.wav");

    for(auto& scroller : scrollers) {
        scroller.m_X = Context::UnsignedRandRange(Board::Width - 1);
        scroller.m_Y = Context::UnsignedRandRange(Board::Height - 1);
        menu_board.Set(scroller.m_X, scroller.m_Y, scroller.m_Piece);
    }

    Tickbox sfx { tickbox_x, tickbox_y, tickbox_scale, true, "SFX_On.png", "SFX_Off.png", ctx, loader.m_Loader };
    sfx.AddChild(Tickbox { tickbox_x + tickbox_scale, tickbox_y, tickbox_scale, false, "Clock.png", "NoClock.png", ctx, loader.m_Loader });

    Button play { play_button_x, play_button_y, play_button_width, play_button_height, "PlayButton.png", ctx, loader.m_Loader };

    ArrowSelect black_piece_select { player_select_inset, player_select_y, player_select_scale, Span<const std::string>(black_piece_paths), ctx, loader.m_Loader };
    ArrowSelect black_weapon_select { player_select_inset, black_piece_select.m_Y + player_select_scale + player_select_item_offset, player_select_scale, Span<const std::string>(weapon_paths), ctx, loader.m_Loader };
    Tickbox black_human { player_select_inset + player_select_scale, black_weapon_select.m_Y + player_select_scale + ai_tickbox_offset, ai_tickbox_scale, true, "Person.png", "Computer.png", ctx, loader.m_Loader };

    ArrowSelect white_piece_select { Context::Width - (player_select_scale * 3) - player_select_inset, player_select_y, player_select_scale, Span<const std::string>(white_piece_paths), ctx, loader.m_Loader };
    ArrowSelect white_weapon_select { white_piece_select.m_X, white_piece_select.m_Y + player_select_scale + player_select_item_offset, player_select_scale, Span<const std::string>(weapon_paths), ctx, loader.m_Loader };
    Tickbox white_human { white_piece_select.m_X + player_select_scale, white_weapon_select.m_Y + player_select_scale + ai_tickbox_offset, ai_tickbox_scale, true, "Person.png", "Computer.png", ctx, loader.m_Loader };

    Dimension x_off = 0;
    Dimension y_off = 0;
    float r = 0;

    title_song.Loop(-1);
    while(true) {
        if(!ctx.Update()) std::exit(0);

        ctx.Clear(Color::DarkGray);

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
            title.Draw(ctx, title_x, title_y, title_width, title_height, rot);
            r += 0.05f;
            if(r >= M_PI * 2) r = 0;
        }

        auto pos = Context::GetMousePosition();
        bool pressed = ctx.WasMousePressed();
        if(sfx.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click) {
            if(sfx.m_State) {
                next_turn.Play();
                title_song.Loop(-1);
            }
            else Context::StopSounds();
        }
        if(clock.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click) next_turn.Play();

        if(black_piece_select.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();
        if(black_weapon_select.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();
        if(black_human.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();

        if(white_piece_select.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();
        if(white_weapon_select.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();
        if(white_human.Update(ctx, pressed, pos.first, pos.second, 0, 0) == UIResult::Click && sfx.m_State) next_turn.Play();

        switch(play.Update(ctx, pressed, pos.first, pos.second, 0, 0)) {
            case UIResult::None: {
                play.m_Rotation = 0;
                break;
            }
            case UIResult::Hover: {
                play.m_Rotation = 2 * rot;
                break;
            }
            case UIResult::Click: goto ret;
        }
    }

    ret:;

    settings.m_MoveTimer = clock.m_State;

    settings.m_WhitePiece = static_cast<Piece>(static_cast<Dimension>(Piece::WhitePawn) + (white_piece_select.m_Current % white_piece_paths.size()));
    settings.m_WhiteWeapon = static_cast<Weapon>(white_weapon_select.m_Current % weapon_paths.size());
    settings.m_WhiteAI = !white_human.m_State;

    settings.m_BlackPiece = static_cast<Piece>(static_cast<Dimension>(Piece::BlackPawn) + (black_piece_select.m_Current % black_piece_paths.size()));
    settings.m_BlackWeapon = static_cast<Weapon>(black_weapon_select.m_Current % weapon_paths.size());
    settings.m_BlackAI = !black_human.m_State;
}
