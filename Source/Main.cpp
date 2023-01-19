#include <stdexcept>
#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <functional>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <spdlog/spdlog.h>

#include <csv2/writer.hpp>

enum class Color {
    Black,
    White,
    Red,
    Green
};

static void SDLResultCheck(int result) {
    if(result < 0) throw std::runtime_error(SDL_GetError());
}

static void SDLNullCheck(void* ptr) {
    if(!ptr) throw std::runtime_error(SDL_GetError());
}

static SDL_Color ColorToSDL(Color color) {
    switch(color) {
        case Color::Black: return {0, 0, 0, 255};
        case Color::White: return {255, 255, 255, 255};
        case Color::Red: return {255, 0, 0, 255};
        case Color::Green: return {0, 255, 0, 255};
    }
}

template<class T>
class SDLHandle {
public:
    T* m_UnderlyingHandle{0};

public:
    SDLHandle() = default;
    SDLHandle(const std::nullptr_t) {};
    SDLHandle(T* handle) : m_UnderlyingHandle(handle) {}

    operator T*() const { return m_UnderlyingHandle; }

    operator bool() const { return m_UnderlyingHandle != nullptr; }
    bool operator!=(const std::nullptr_t) const { return m_UnderlyingHandle != 0; }
};

template<class T, void (*func)(T*)>
class SDLDestructor {
public:
    using pointer = SDLHandle<T>;

public:
    void operator()(SDLHandle<T> handle) const {
        func(handle);
    }
};

using Dimension = int;
static constexpr Dimension DimensionMax = INT_MAX;
static constexpr Dimension DimensionMin = INT_MIN;

static bool IsPointInRect(Dimension px, Dimension py, Dimension rx, Dimension ry, Dimension rw, Dimension rh) {
    return px >= rx && px <= (rx + rw) && py >= ry && py <= (ry + rh);
}

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
    BlackQueen
};

struct PieceMove {
    Dimension m_Dx;
    Dimension m_Dy;
    bool m_Fill;
};

static std::vector<PieceMove> EnumeratePieceMoves(Piece piece) {
    switch(piece) {
        case Piece::None: return {};

        case Piece::WhitePawn: return {{0, -1}};
        case Piece::BlackPawn: return {{0, 1}};

        case Piece::WhiteRook: [[fallthrough]];
        case Piece::BlackRook: return {{0, DimensionMax, true}, {0, DimensionMin, true}, {DimensionMax, 0, true}, {DimensionMin, 0, true}};

        case Piece::WhiteBishop: [[fallthrough]];
        case Piece::BlackBishop: return {{DimensionMax, DimensionMax, true}, {DimensionMax, DimensionMin, true}, {DimensionMin, DimensionMax, true}, {DimensionMin, DimensionMin, true}};

        case Piece::WhiteKnight: [[fallthrough]];
        case Piece::BlackKnight: return {{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {-2, 1}, {2, -1}, {-2, -1}};

        case Piece::WhiteKing: [[fallthrough]];
        case Piece::BlackKing: return {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};

        case Piece::WhiteQueen: [[fallthrough]];
        case Piece::BlackQueen: return {{0, DimensionMax, true}, {DimensionMax, DimensionMax, true}, {DimensionMax, 0, true}, {DimensionMax, DimensionMin, true}, {0, DimensionMin, true}, {DimensionMin, DimensionMin, true}, {DimensionMin, 0, true}, {DimensionMin, DimensionMax, true}};
    }
}

class Texture;

class Context {
private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;

    std::unordered_map<SDL_KeyCode, bool> m_KeyStates;
    bool m_MouseHeld{};

    friend class Texture;

public:
    Context() {
        int result = SDL_Init(SDL_INIT_EVERYTHING);
        if(result < 0) throw std::runtime_error("Could not init SDL2");

        result = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF);
        if(result < 0) throw std::runtime_error("Could not init SDL2_image");

        SDLNullCheck(m_Window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));
        SDLNullCheck(m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    }

    ~Context() {
        SDL_DestroyWindow(m_Window);
        SDL_DestroyRenderer(m_Renderer);
        SDL_Quit();
    }

    bool Update() {
        SDL_RenderPresent(m_Renderer);

        SDL_Event event{};
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: return false;
                case SDL_KEYDOWN: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = true; break;
                case SDL_KEYUP: m_KeyStates[(SDL_KeyCode) event.key.keysym.sym] = false; break;
                case SDL_MOUSEBUTTONDOWN: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = true; break;
                case SDL_MOUSEBUTTONUP: if(event.button.button == SDL_BUTTON_LEFT) m_MouseHeld = false; break;
                default: break;
            }
        }

        return true;
    }

    void SetColor(Color color) {
        SDL_Color sdl_color = ColorToSDL(color);
        SDLResultCheck(SDL_SetRenderDrawColor(m_Renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    }

    void Clear(Color color) {
        SetColor(color);
        SDLResultCheck(SDL_RenderClear(m_Renderer));
    }

    void DrawRect(Dimension x, Dimension y, Dimension w, Dimension h, Color color) {
        SetColor(color);

        SDL_Rect rect {x, y, w, h};
        SDLResultCheck(SDL_RenderFillRect(m_Renderer, &rect));
    }

    bool IsKeyHeld(SDL_KeyCode keycode) {
        return m_KeyStates[keycode];
    }

    bool WasKeyPressed(SDL_KeyCode keycode) {
        bool pressed = IsKeyHeld(keycode);
        m_KeyStates[keycode] = false;
        return pressed;
    }

    bool IsMouseHeld() {
        return m_MouseHeld;
    }

    bool WasMousePressed() {
        bool pressed = IsMouseHeld();
        m_MouseHeld = false;
        return pressed;
    }

    std::pair<Dimension, Dimension> GetMousePosition() {
        std::pair<Dimension, Dimension> ret;
        SDL_GetMouseState(&ret.first, &ret.second);
        return ret;
    }
};

class Texture {
private:
    static void Deleter(SDL_Texture* texture) { spdlog::debug("Destroying texture @{}", (void*) texture); SDL_DestroyTexture(texture); };
    using Handle = std::unique_ptr<SDLHandle<SDL_Texture>, SDLDestructor<SDL_Texture, Deleter>>;

private:
    Handle m_Texture{};
    Dimension m_Width{};
    Dimension m_Height{};

    bool m_Dummy;

public:
    Texture() : m_Dummy(true) {};

    Texture(const std::string& path, Context& ctx) : m_Dummy(false) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        SDLNullCheck(surface);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ctx.m_Renderer, surface);
        SDLNullCheck(texture);
        m_Texture.reset(texture);

        m_Width = surface->w;
        m_Height = surface->h;

        SDL_FreeSurface(surface);
    }

    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height) {
        if(!m_Dummy) {
            SDL_Rect src {0, 0, m_Width, m_Height};
            SDL_Rect dest {x, y, width, height};
            SDLResultCheck(SDL_RenderCopy(ctx.m_Renderer, m_Texture.get(), &src, &dest));
        }
    }

    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height, float rotation) {
        if(!m_Dummy) {
            SDL_Rect src {0, 0, m_Width, m_Height};
            SDL_Rect dest {x, y, width, height};
            SDLResultCheck(SDL_RenderCopyEx(ctx.m_Renderer, m_Texture.get(), &src, &dest, rotation, nullptr, SDL_FLIP_NONE));
        }
    }
};

template<class T, Dimension ResourcePoolSize>
class ResourceLoader {
private:
    std::string m_ResourceDirectory;
    std::array<T, ResourcePoolSize> m_Resources;
    Dimension m_ResourcesLast{0};
    std::unordered_map<std::string, Dimension> m_Map;

public:
    explicit ResourceLoader(std::string resource_directory) : m_ResourceDirectory(std::move(resource_directory)) {}

    T& Get(const std::string& path, Context& ctx) {
        auto emplaced = m_Map.try_emplace(path, ResourcePoolSize);
        auto it = emplaced.first;
        auto added = emplaced.second;

        if(added) {
            std::string fullpath = m_ResourceDirectory + "/" + path;
            spdlog::debug("Loaded resource from path '{}'", fullpath);
            m_Resources[m_ResourcesLast] = std::move(T(fullpath, ctx));
            it->second = m_ResourcesLast++;
        }

        return m_Resources[it->second];
    }
};

using TextureLoader = ResourceLoader<Texture, 1024>;

class Board {
public:
    constexpr static Dimension SquareScale = 56;

private:
    constexpr static Dimension Width = 8;
    constexpr static Dimension Height = 8;

    std::array<Piece, Width * Height> m_Board{};

    Texture m_NoneDummy;
    std::unordered_map<Piece, std::reference_wrapper<Texture>> m_PieceTextures;

public:
    static bool IsInBounds(Dimension x, Dimension y) {
        return !(x < 0 || y < 0 || x >= Board::Width || y >= Board::Height);
    }

    Board(TextureLoader& loader, Context& ctx) : m_Board{}, m_NoneDummy(), m_PieceTextures{} {
        m_PieceTextures.insert({Piece::None, m_NoneDummy});

        m_PieceTextures.insert({Piece::WhitePawn, loader.Get("WhitePawn.png", ctx)});
        m_PieceTextures.insert({Piece::WhiteRook, loader.Get("WhiteRook.png", ctx)});
        m_PieceTextures.insert({Piece::WhiteBishop, loader.Get("WhiteBishop.png", ctx)});
        m_PieceTextures.insert({Piece::WhiteKnight, loader.Get("WhiteKnight.png", ctx)});
        m_PieceTextures.insert({Piece::WhiteKing, loader.Get("WhiteKing.png", ctx)});
        m_PieceTextures.insert({Piece::WhiteQueen, loader.Get("WhiteQueen.png", ctx)});

        m_PieceTextures.insert({Piece::BlackPawn, loader.Get("BlackPawn.png", ctx)});
        m_PieceTextures.insert({Piece::BlackRook, loader.Get("BlackRook.png", ctx)});
        m_PieceTextures.insert({Piece::BlackBishop, loader.Get("BlackBishop.png", ctx)});
        m_PieceTextures.insert({Piece::BlackKnight, loader.Get("BlackKnight.png", ctx)});
        m_PieceTextures.insert({Piece::BlackKing, loader.Get("BlackKing.png", ctx)});
        m_PieceTextures.insert({Piece::BlackQueen, loader.Get("BlackQueen.png", ctx)});
    }

    void Draw(Context& ctx) {
        for(Dimension i = 0; i < Height; ++i) {
            for(Dimension j = 0; j < Width; ++j) {
                ctx.DrawRect(j * SquareScale, i * SquareScale, SquareScale, SquareScale, (j + i % 2) % 2 ? Color::Black : Color::White);
                m_PieceTextures.at(m_Board[j + i * Width]).get().Draw(ctx, j * SquareScale, i * SquareScale, SquareScale, SquareScale);
            }
        }
    }

    void Set(Dimension x, Dimension y, Piece piece) {
        m_Board[x + Width * y] = piece;
    }

    Piece Get(Dimension x, Dimension y) {
        return m_Board[x + Width * y];
    }
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

static float WeaponDamage(Weapon weapon) {
    switch(weapon) {
        case Weapon::None: return 0.0f;
        case Weapon::AimTest: return 9.0f;
        case Weapon::Pistol: return 7.0f;
        case Weapon::Shotgun: return 4.0f;
        case Weapon::ScienceGun: return 9.0f;
        case Weapon::Rifle: return 9.0f;
        case Weapon::RocketLauncher: return 35.0f;
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
        case Weapon::AimTest: return 3;
        case Weapon::Pistol: return 1;
        case Weapon::Shotgun: return 7;
        case Weapon::ScienceGun: return 3;
        case Weapon::Rifle: return 1;
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
    Piece DoMove(Context& ctx, Board& board, Piece ignore) {
        if(m_Shown) {
            float dx = m_Speed * cos(m_Rotation);
            float dy = m_Speed * sin(m_Rotation);
            m_X += dx;
            m_Y += dy;

            Dimension x = static_cast<Dimension>(m_X);
            Dimension y = static_cast<Dimension>(m_Y);

            ctx.DrawRect(x, y, ProjectileScale, ProjectileScale, Color::Red);

            if(!Board::IsInBounds(x / Board::SquareScale, y / Board::SquareScale)) {
                m_Shown = false;
                return Piece::None;
            }

            Piece piece = board.Get(x / Board::SquareScale, y / Board::SquareScale);
            if(piece != Piece::None && piece != ignore) {
                m_Shown = false;
                return piece;
            }
        }

        return Piece::None;
    }
};

static float SignedRandRange(float range) {
    return ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (2 * range)) - range;
}

class Player {
public:
    static constexpr float MaxHealth = 100.0f;

private:
    static constexpr Dimension IndicatorScale = Board::SquareScale / 2;
    static constexpr float ProjectileSpeed = 10.0f;
    static constexpr Dimension MaxProjectiles = 10;

    Piece m_Piece;
    Weapon m_Weapon;

public:
    Dimension m_X;
    Dimension m_Y;

    float m_Health{MaxHealth};
    float m_Damage{};

    Dimension m_Turns;
    std::vector<float> m_HealthPerTurn;
    std::vector<float> m_DamagePerTurn;
    std::vector<float> m_DistancePerTurn;

public:
    std::array<Projectile, MaxProjectiles> m_Projectiles{};

public:
    Player(Piece piece, Weapon weapon) : m_X(0), m_Y(0), m_Piece(piece), m_Weapon(weapon) {};

    void Move(Board& board, Dimension dx, Dimension dy) {
        board.Set(m_X, m_Y, Piece::None);

        m_X += dx;
        m_Y += dy;
        if(!Board::IsInBounds(m_X, m_Y)) throw std::runtime_error("Attempt to move player out of bounds");

        board.Set(m_X, m_Y, m_Piece);

        spdlog::debug("Moved from ({},{}) to ({},{})", m_X - dx, m_Y - dy, m_X, m_Y);
    }

    std::vector<std::pair<Dimension, Dimension>> EnumerateValidPositions(Board& board) {
        auto piece_moves = EnumeratePieceMoves(m_Piece);
        spdlog::trace("Generated {} candidate moves", piece_moves.size());

        std::vector<std::pair<Dimension, Dimension>> positions{};

        for(PieceMove& move : piece_moves) {
            if(!move.m_Fill) {
                if(board.Get(m_X + move.m_Dx, m_Y + move.m_Dy) == Piece::None) {
                    positions.emplace_back(move.m_Dx, move.m_Dy);
                    spdlog::trace("Added non-filling move ({},{})", move.m_Dx, move.m_Dy);
                }
            }
            else {
                Dimension dx = 0;
                Dimension dy = 0;
                spdlog::trace("Adding filled moves in bounds +(0,0) -> +({},{})...", move.m_Dx, move.m_Dy);
                while(true) {
//                    dx += sign(move.m_Dx)

                    if(move.m_Dx > 0 && dx < move.m_Dx) ++dx;
                    else if(move.m_Dx < 0 && dx > move.m_Dx) --dx;
                    else if(move.m_Dx) {
                        spdlog::trace("Early finalize: {} == {}", dx, move.m_Dx);
                        break;
                    }

                    if(move.m_Dy > 0 && dy < move.m_Dy) ++dy;
                    else if(move.m_Dy < 0 && dy > move.m_Dy) --dy;
                    else if(move.m_Dy) {
                        spdlog::trace("Early finalize: {} == {}", dy, move.m_Dy);
                        break;
                    }

                    if(!Board::IsInBounds(m_X + dx, m_Y + dy)) {
                        spdlog::trace("Early finalize: ({},{}) (from delta +({},{})) out of bounds", m_X + dx, m_Y + dy, dx, dy);
                        break;
                    }

                    if(board.Get(m_X + dx, m_Y + dy) != Piece::None) {
                        spdlog::trace("Early finalize: Piece blocking at ({},{})", m_X + dx, m_Y + dy);
                        break;
                    }

                    positions.emplace_back(dx, dy);
                    spdlog::trace("Added filled move ({},{})", dx, dy);
                }
            }
        }

        return positions;
    }

    bool DoMoves(Context& ctx, Board& board) {
        for(auto& position : EnumerateValidPositions(board)) {
            if(!Board::IsInBounds(m_X + position.first, m_Y + position.second)) continue;

            ctx.DrawRect((m_X + position.first) * Board::SquareScale, (m_Y + position.second) * Board::SquareScale, IndicatorScale, IndicatorScale, Color::Green);

            auto pos = ctx.GetMousePosition();
            if(IsPointInRect(pos.first, pos.second, (m_X + position.first) * Board::SquareScale, (m_Y + position.second) * Board::SquareScale, Board::SquareScale, Board::SquareScale)) {
                if(ctx.WasMousePressed()) {
                    Move(board, position.first, position.second);
                    return true;
                }
            }
        }
        return false;
    }

    bool DoWeapon(Context& ctx, WeaponTextures& textures) {
        auto pos = ctx.GetMousePosition();
        float rot = atan(static_cast<float>(pos.second - m_Y * Board::SquareScale) / static_cast<float>(pos.first - m_X * Board::SquareScale));
        textures.m_Textures.at(m_Weapon).get().Draw(ctx, m_X * Board::SquareScale, m_Y * Board::SquareScale, Board::SquareScale, Board::SquareScale, (rot * 180.0f) / static_cast<float>(M_PI));

        if(ctx.WasMousePressed()) {
            rot += pos.first - m_X * Board::SquareScale < 0 ? M_PI : 0;
            for(Dimension i = 0; i < WeaponCount(m_Weapon); ++i) {
                for(Projectile& projectile : m_Projectiles) {
                    if(!projectile.m_Shown) {
                        projectile = Projectile{static_cast<float>(m_X * Board::SquareScale), static_cast<float>(m_Y * Board::SquareScale), rot + SignedRandRange(WeaponSpread(m_Weapon)), ProjectileSpeed, true};
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

using Row = std::array<std::string, 5>;

static void WriteStats(Player& black, Player& white) {
    std::ofstream stream("game.csv");
    csv2::Writer<csv2::delimiter<','>> writer(stream);

    std::vector<Row> rows {};

    rows.emplace_back(Row{"BlackHealthPerTurn", "BlackDamagePerTurn", "WhiteHealthPerTurn", "WhiteDamagePerTurn", "DistancePerTurn"});

    Dimension min_turns = black.m_Turns < white.m_Turns ? black.m_Turns : white.m_Turns;
    for(Dimension i = 0; i < min_turns; ++i) {
        rows.emplace_back(Row{fmt::to_string(black.m_HealthPerTurn[i]), fmt::to_string(black.m_DamagePerTurn[i]), fmt::to_string(white.m_HealthPerTurn[i]), fmt::to_string(white.m_DamagePerTurn[i]), fmt::to_string(black.m_DistancePerTurn[i])});
    }

    writer.write_rows(rows);
    stream.close();
}

int main() {
    srand(time(nullptr));

    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initializing CWG, hold onto your hats...");

    Context ctx{};
    TextureLoader loader("Resources");
    Board board(loader, ctx);
    WeaponTextures weapon_textures(loader, ctx);

    SDL_MessageBoxButtonData weapon_buttons[] {
        {0, (int) Weapon::None, "None"},
        {0, (int) Weapon::AimTest, "Aim Test"},
        {0, (int) Weapon::Pistol, "Pistol"},
        {0, (int) Weapon::Shotgun, "Shotgun"},
        {0, (int) Weapon::ScienceGun, "Science Gun"},
        {0, (int) Weapon::Rifle, "Rifle"},
        {0, (int) Weapon::RocketLauncher, "Rocket Launcher"}
    };

    Piece white_piece = Piece::None;
    SDL_MessageBoxButtonData white_piece_buttons[] {
        {0, (int) Piece::WhitePawn, "Pawn"},
        {0, (int) Piece::WhiteRook, "Rook"},
        {0, (int) Piece::WhiteBishop, "Bishop"},
        {0, (int) Piece::WhiteKnight, "Knight"},
        {0, (int) Piece::WhiteKing, "King"},
        {0, (int) Piece::WhiteQueen, "Queen"}
    };
    SDL_MessageBoxData white_piece_data{SDL_MESSAGEBOX_INFORMATION, nullptr, "White Piece", "White, please choose your piece!", sizeof(white_piece_buttons) / sizeof(white_piece_buttons[0]), white_piece_buttons, nullptr};
    SDLResultCheck(SDL_ShowMessageBox(&white_piece_data, (int*) &white_piece));

    Weapon white_weapon = Weapon::None;
    SDL_MessageBoxData white_weapon_data{SDL_MESSAGEBOX_INFORMATION, nullptr, "White Weapon", "White, please choose your weapon!", sizeof(weapon_buttons) / sizeof(weapon_buttons[0]), weapon_buttons, nullptr};
    SDLResultCheck(SDL_ShowMessageBox(&white_weapon_data, (int*) &white_weapon));

    Player white(white_piece, white_weapon);
    white.Move(board, 7, 7);

    Piece black_piece = Piece::None;
    SDL_MessageBoxButtonData black_piece_buttons[] {
        {0, (int) Piece::BlackPawn, "Pawn"},
        {0, (int) Piece::BlackRook, "Rook"},
        {0, (int) Piece::BlackBishop, "Bishop"},
        {0, (int) Piece::BlackKnight, "Knight"},
        {0, (int) Piece::BlackKing, "King"},
        {0, (int) Piece::BlackQueen, "Queen"}
    };
    SDL_MessageBoxData black_piece_data{SDL_MESSAGEBOX_INFORMATION, nullptr, "Black Piece", "Black, please choose your piece!", sizeof(black_piece_buttons) / sizeof(black_piece_buttons[0]), black_piece_buttons, nullptr};
    SDLResultCheck(SDL_ShowMessageBox(&black_piece_data, (int*) &black_piece));

    Weapon black_weapon = Weapon::None;
    SDL_MessageBoxData black_weapon_data{SDL_MESSAGEBOX_INFORMATION, nullptr, "Black Weapon", "Black, please choose your weapon!", sizeof(weapon_buttons) / sizeof(weapon_buttons[0]), weapon_buttons, nullptr};
    SDLResultCheck(SDL_ShowMessageBox(&black_weapon_data, (int*) &black_weapon));

    Player black(black_piece, black_weapon);
    black.Move(board, 0, 0);

    bool white_turn = true;

    while(ctx.Update()) {
        ctx.Clear(Color::Red);
        board.Draw(ctx);

        if(white_turn) {
            if(white.DoMoves(ctx, board) || white.DoWeapon(ctx, weapon_textures)) {
                white.m_Turns++;
                white.m_HealthPerTurn.emplace_back(white.m_Health);
                white.m_DamagePerTurn.emplace_back(white.m_Damage);
                float dx = static_cast<float>(white.m_X - black.m_X);
                float dy = static_cast<float>(white.m_Y - black.m_Y);
                float dist = sqrtf(dx*dx + dx*dy);
                spdlog::debug("Position delta +({},{}) -> dist: {}", dx, dy, dist);
                white.m_DistancePerTurn.emplace_back(dist);
                white_turn = false;
            }
        }
        else {
            if(black.DoMoves(ctx, board) || black.DoWeapon(ctx, weapon_textures)) {
                black.m_Turns++;
                black.m_HealthPerTurn.emplace_back(black.m_Health);
                black.m_DamagePerTurn.emplace_back(black.m_Damage);
                float dx = static_cast<float>(white.m_X - black.m_X);
                float dy = static_cast<float>(white.m_Y - black.m_Y);
                float dist = sqrtf(dx*dx + dx*dy);
                spdlog::debug("Position delta +({},{}) -> dist: {}", dx, dy, dist);
                black.m_DistancePerTurn.emplace_back(dist);
                white_turn = true;
            }
        }

        for(Projectile& projectile : white.m_Projectiles) {
            Piece hit = projectile.DoMove(ctx, board, white_piece);
            if(hit != Piece::None) {
                spdlog::debug("White projectile hit - Black HP: {}", black.m_Health);
                projectile.m_Shown = false;
                float damage = WeaponDamage(white_weapon) + SignedRandRange(WeaponVariance(white_weapon));
                white.m_Damage += damage;
                if(black.Hurt(damage)) {
                    SDLResultCheck(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "White won!", nullptr));
                    WriteStats(black, white);
                    return 0;
                }
            }
        }
        for(Projectile& projectile : black.m_Projectiles) {
            Piece hit = projectile.DoMove(ctx, board, black_piece);
            if(hit != Piece::None) {
                spdlog::debug("Black projectile hit - White HP: {}", white.m_Health);
                projectile.m_Shown = false;
                float damage = WeaponDamage(black_weapon) + SignedRandRange(WeaponVariance(black_weapon));
                black.m_Damage += damage;
                if(white.Hurt(damage)) {
                    SDLResultCheck(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "Black won!", nullptr));
                    WriteStats(black, white);
                    return 0;
                }
            }
        }

        ctx.DrawRect(Board::SquareScale * 8, 0, static_cast<float>(640 - Board::SquareScale * 8) * (black.m_Health / Player::MaxHealth), 32, Color::Black);
        ctx.DrawRect(Board::SquareScale * 8, 480 - 32, static_cast<float>(640 - Board::SquareScale * 8) * (white.m_Health / Player::MaxHealth), 32, Color::White);
    }

    spdlog::info("Tearing down...");
}
