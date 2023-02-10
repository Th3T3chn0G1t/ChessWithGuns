#include <Util.hpp>
#include <FX.hpp>
#include <CWG.hpp>
#include <Context.hpp>
#include <Texture.hpp>
#include <Elements.hpp>
#include <Player.hpp>
#include <SoundEffect.hpp>

int main() {
    Context ctx{};
    TextureLoaderWrapper loader(TextureLoader("Resources"));
    SoundEffectLoader sfx_loader("Resources");
    Board board(loader, ctx);
    WeaponTextures weapon_textures(loader, ctx);
    SoundEffects sound_effects(sfx_loader);

    Context::DialogChoices<Weapon> weapons {
        {Weapon::None, "None"},
        {Weapon::AimTest, "Aim Test"},
        {Weapon::Pistol, "Pistol"},
        {Weapon::Shotgun, "Shotgun"},
        {Weapon::ScienceGun, "Science Gun"},
        {Weapon::Rifle, "Rifle"},
        {Weapon::RocketLauncher, "Rocket Launcher"}
    };

    std::array<Player, 2> players {
        Player{
            Context::ChoiceDialog<Piece>({
                { Piece::WhitePawn, "Pawn" },
                { Piece::WhiteRook, "Rook"},
                { Piece::WhiteBishop, "Bishop"},
                { Piece::WhiteKnight, "Knight"},
                { Piece::WhiteKing, "King"},
                { Piece::WhiteQueen, "Queen"}
            }, "White Piece", "White, please choose your piece!"),
            Context::ChoiceDialog<Weapon>(weapons, "White Weapon", "White, please choose your weapon!"),
            Context::ChoiceDialog("White AI", "Should White be AI-controlled?"),
            Board::Width - 1, Board::Height - 1, board,
            "White", Color::White
        },
        Player{
            Context::ChoiceDialog<Piece>({
                { Piece::BlackPawn, "Pawn" },
                { Piece::BlackRook, "Rook"},
                { Piece::BlackBishop, "Bishop"},
                { Piece::BlackKnight, "Knight"},
                { Piece::BlackKing, "King"},
                { Piece::BlackQueen, "Queen"}
            }, "Black Piece", "Black, please choose your piece!"),
            Context::ChoiceDialog<Weapon>(weapons, "Black Weapon", "Black, please choose your weapon!"),
            Context::ChoiceDialog("Black AI", "Should Black be AI-controlled?"),
            0, 0, board,
            "Black", Color::Black
        }
    };

    bool do_sfx = Context::ChoiceDialog("SFX", "Should sound effects be enabled? (Not recommended for simulation play)");

    std::array<Pickup, 2> pickups{
        Pickup{board},
        Pickup{board}
    };

    Dimension turn = 0;
    Dimension dead = 0;

    SoundEffect& next_turn = sfx_loader.Get("Turn.wav");

    while(ctx.Update()) {
        ctx.Clear(Color::Gray);
        board.Draw(ctx);

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
                                Context::Dialog("Death", other.m_Name + " died");
                                other.m_Dead = true;
                                board.Set(other.m_X, other.m_Y, Piece::None);
                                dead++;
                                if(dead >= players.size() - 1) {
                                    Context::Dialog("Game Over", fired.m_Name + " won!");
                                    return 0;
                                }
                            }
                            goto ui;
                        }
                    }
                    Context::Dialog("Error", "Invalid Hit");
                    return 1;
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
