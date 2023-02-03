#include <Util.hpp>
#include <FX.hpp>
#include <CWG.hpp>
#include <Context.hpp>
#include <Texture.hpp>

enum class Weapon {
    None,

    AimTest,

    Pistol,
    Shotgun,
    ScienceGun,
    Rifle,
    RocketLauncher
};

static float WeaponDamage(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0.0f;
        case Weapon::AimTest: return 11.0f;
        case Weapon::Pistol: return 9.0f;
        case Weapon::Shotgun: return 8.0f;
        case Weapon::ScienceGun: return 5.0f;
        case Weapon::Rifle: return 11.0f;
        case Weapon::RocketLauncher: return 37.0f;
    }
}

static float WeaponSpread(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0.0f;
        case Weapon::AimTest: return ((15.0f * static_cast<float>(M_PI)) / 180.0f);
        case Weapon::Pistol: return ((10.0f * static_cast<float>(M_PI)) / 180.0f);
        case Weapon::Shotgun: return ((35.0f * static_cast<float>(M_PI)) / 180.0f);
        case Weapon::ScienceGun: return ((15.0f * static_cast<float>(M_PI)) / 180.0f);
        case Weapon::Rifle: return ((5.0f * static_cast<float>(M_PI)) / 180.0f);
        case Weapon::RocketLauncher: return ((35.0f * static_cast<float>(M_PI)) / 180.0f);
    }
}

static float WeaponVariance(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0.0f;
        case Weapon::AimTest: return 4.0f;
        case Weapon::Pistol: return 2.0f;
        case Weapon::Shotgun: return 1.0f;
        case Weapon::ScienceGun: return 3.0f;
        case Weapon::Rifle: return 4.0f;
        case Weapon::RocketLauncher: return 10.0f;
    }
}

static Dimension WeaponCount(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0;
        case Weapon::AimTest: return 300;
        case Weapon::Pistol: return 1;
        case Weapon::Shotgun: return 7;
        case Weapon::ScienceGun: return 3;
        case Weapon::Rifle: return 2;
        case Weapon::RocketLauncher: return 1;
    }
}

static Dimension WeaponAmmo(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0;
        case Weapon::AimTest: return 6;
        case Weapon::Pistol: return 20;
        case Weapon::Shotgun: return 15;
        case Weapon::ScienceGun: return 6;
        case Weapon::Rifle: return 10;
        case Weapon::RocketLauncher: return 1;
    }
}

class WeaponTextures {
public:
    Texture m_NoneDummy;
    std::unordered_map<Weapon, std::reference_wrapper<Texture>> m_Textures;

    WeaponTextures(TextureLoader& loader, Context& ctx) : m_NoneDummy() {
        m_Textures.insert({Weapon::None, m_NoneDummy});

        m_Textures.insert({Weapon::AimTest, loader.Get("AimTest.png", ctx)});
        m_Textures.insert({Weapon::Pistol, loader.Get("Pistol.png", ctx)});
        m_Textures.insert({Weapon::Shotgun, loader.Get("Shotgun.png", ctx)});
        m_Textures.insert({Weapon::ScienceGun, loader.Get("ScienceGun.png", ctx)});
        m_Textures.insert({Weapon::Rifle, loader.Get("Rifle.png", ctx)});
        m_Textures.insert({Weapon::RocketLauncher, loader.Get("RocketLauncher.png", ctx)});
    }
};

bool IsPickup(Piece piece) {
    return piece == Piece::AmmoPickup || piece == Piece::HealthPickup || piece == Piece::BoostPickup;
}

class Projectile {
private:
    static constexpr Dimension ProjectileScale = 4;

public:
    float m_X;
    float m_Y;
    float m_Rotation;
    float m_Speed;
    bool m_Shown;

public:
    Piece DoMove(Context& ctx, Board& board, Piece ignore, bool boosted) {
        if(m_Shown) {
            float dx = m_Speed * cos(m_Rotation);
            float dy = m_Speed * sin(m_Rotation);
            m_X += dx;
            m_Y += dy;

            auto x = static_cast<Dimension>(m_X);
            auto y = static_cast<Dimension>(m_Y);

            ctx.DrawRect(x, y, ProjectileScale, ProjectileScale, boosted ? Color::Blue : Color::Red);

            if(!Board::IsInBounds(x / Board::SquareScale, y / Board::SquareScale)) {
                m_Shown = false;
                return Piece::None;
            }

            Piece piece = board.Get(x / Board::SquareScale, y / Board::SquareScale);
            if(piece != Piece::None && piece != ignore && !IsPickup(piece)) {
                m_Shown = false;
                return piece;
            }
        }

        return Piece::None;
    }
};

class Pickup {
public:
    Dimension m_X;
    Dimension m_Y;

    explicit Pickup(Board& board) {
        do {
            m_X = Context::UnsignedRandRange(Board::Width - 1);
            m_Y = Context::UnsignedRandRange(Board::Height - 1);
        } while(board.Get(m_X, m_Y) != Piece::None);

        if(Context::UnsignedRandRange(3)) board.Set(m_X, m_Y, Piece::AmmoPickup);
        else if(Context::UnsignedRandRange(2)) board.Set(m_X, m_Y, Piece::BoostPickup);
        else board.Set(m_X, m_Y, Piece::HealthPickup);
    }

    void Place(Board& board) {
        Dimension x = m_X;
        Dimension y = m_Y;

        do {
            m_X = Context::UnsignedRandRange(Board::Width - 1);
            m_Y = Context::UnsignedRandRange(Board::Height - 1);
        } while(board.Get(m_X, m_Y) != Piece::None);

        if(Context::UnsignedRandRange(3)) board.Set(m_X, m_Y, Piece::AmmoPickup);
        else if(Context::UnsignedRandRange(2)) board.Set(m_X, m_Y, Piece::BoostPickup);
        else board.Set(m_X, m_Y, Piece::HealthPickup);

        board.Set(x, y, Piece::None);
    }
};

class Player {
public:
    static constexpr float MaxHealth = 100.0f;

private:
    static constexpr Dimension IndicatorScale = Board::SquareScale / 2;
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
    Player(Piece piece, Weapon weapon, bool ai, Dimension x, Dimension y, Board& board, std::string  name, Color color) : m_X(0), m_Y(0), m_Piece(piece), m_Weapon(weapon), m_AI(ai), m_Name(std::move(name)), m_Color(color) {
        board.Set(m_X, m_Y, Piece::None);
        Move(board, x, y);
        m_Ammo = WeaponAmmo(weapon);
    };

    void Move(Board& board, Dimension dx, Dimension dy) {
        board.Set(m_X, m_Y, Piece::None);

        m_X += dx;
        m_Y += dy;
        if(!Board::IsInBounds(m_X, m_Y)) throw std::runtime_error("Attempt to move player out of bounds");

        board.Set(m_X, m_Y, m_Piece);
    }

    std::vector<std::pair<Dimension, Dimension>> EnumerateValidPositions(Board& board) const {
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

    void PickupCheck(Board& board, Dimension x, Dimension y, Span<Pickup> pickups) {
        Piece at = board.Get(x, y);
        if(at == Piece::AmmoPickup) {
            m_Ammo += 5;
            if(m_Ammo > WeaponAmmo(m_Weapon)) m_Ammo = WeaponAmmo(m_Weapon);
        }
        else if(at == Piece::HealthPickup) {
            m_Health += 7;
            if(m_Health > MaxHealth) m_Health = MaxHealth;
        }
        else if(at == Piece::BoostPickup) {
            m_DamageBoost = 5;
        }

        if(IsPickup(at)) {
            for(size_t i = 0; i < pickups.m_Size; ++i) {
                if(pickups.m_Data[i].m_X == x && pickups.m_Data[i].m_Y == y) {
                    pickups.m_Data[i].Place(board);
                    return;
                }
            }
            Context::Dialog("Error", "Invalid Pickup at " + std::to_string(x) + " " + std::to_string(y));
        }
    }

    bool DoMoves(Context& ctx, Board& board, Span<Pickup> pickups) {
        auto positions = EnumerateValidPositions(board);
        if(!m_AI) {
            for(auto& position : positions) {
                Dimension new_x = m_X + position.first;
                Dimension new_y = m_Y + position.second;

                if(!Board::IsInBounds(new_x, new_y)) continue;

                ctx.DrawRect(new_x * Board::SquareScale, new_y * Board::SquareScale, IndicatorScale, IndicatorScale, Color::Green);

                auto pos = Context::GetMousePosition();

                if(IsPointInRect(pos.first, pos.second, new_x * Board::SquareScale, new_y * Board::SquareScale, Board::SquareScale, Board::SquareScale)) {
                    if(ctx.WasMousePressed()) {
                        PickupCheck(board, new_x, new_y, pickups);

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
                    PickupCheck(board, m_X + position.first, m_Y + position.second, pickups);

                    Move(board, position.first, position.second);
                    return true;
                }
            }

            auto& position = positions[Context::UnsignedRandRange((int) positions.size())];
            if(!Board::IsInBounds(m_X + position.first, m_Y + position.second)) return false;

            PickupCheck(board, m_X + position.first, m_Y + position.second, pickups);

            Move(board, position.first, position.second);
            return true;
        }

        return false;
    }

    bool DoWeapon(Context& ctx, WeaponTextures& textures, Span<Player> players) {
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
                for(Dimension i = 0; i < WeaponCount(m_Weapon); ++i) {
                    for(Projectile& projectile : m_Projectiles) {
                        if(!projectile.m_Shown) {
                            projectile = Projectile{static_cast<float>(m_X * Board::SquareScale), static_cast<float>(m_Y * Board::SquareScale), rot + Context::SignedRandRange(WeaponSpread(m_Weapon)), ProjectileSpeed, true};
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
            for(Dimension i = 0; i < WeaponCount(m_Weapon); ++i) {
                for(Projectile& projectile : m_Projectiles) {
                    if(!projectile.m_Shown) {
                        projectile = Projectile{static_cast<float>(m_X * Board::SquareScale), static_cast<float>(m_Y * Board::SquareScale), rot + Context::SignedRandRange(WeaponSpread(m_Weapon)), ProjectileSpeed, true};
                        break;
                    }
                }
            }
            return true;
        }

        return false;
    }

    bool Hurt(float damage) {
        m_Health -= damage;
        return m_Health <= 0.0f;
    }
};

int main() {
    Context ctx{};
    TextureLoaderWrapper loader(TextureLoader("Resources"));
    Board board(loader, ctx);
    WeaponTextures weapon_textures(loader.m_Loader, ctx);

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

    std::array<Pickup, 2> pickups{
        Pickup{board},
        Pickup{board}
    };

    Dimension turn = 0;
    Dimension dead = 0;

    while(ctx.Update()) {
        ctx.Clear(Color::Gray);
        board.Draw(ctx);

        auto& player = players[turn];
        if(player.m_Dead) {
            if(++turn >= players.size()) turn = 0;
            goto ui;
        }

        if(player.DoMoves(ctx, board, Span<Pickup>(pickups)) || player.DoWeapon(ctx, weapon_textures, Span<Player>(players))) {
            if(++turn >= players.size()) turn = 0;
        }

        for(auto& fired : players) {
            for(Projectile& projectile : fired.m_Projectiles) {
                Piece hit = projectile.DoMove(ctx, board, fired.m_Piece, !!fired.m_DamageBoost);
                if(hit != Piece::None) {
                    projectile.m_Shown = false;
                    float damage = WeaponDamage(fired.m_Weapon) + Context::SignedRandRange(WeaponVariance(fired.m_Weapon)) + static_cast<float>(fired.m_DamageBoost);
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
