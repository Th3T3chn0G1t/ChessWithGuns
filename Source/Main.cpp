#include <memory>
#include <stdexcept>
#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <random>
#include <cmath>
#include <climits>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using Dimension = int;
static constexpr Dimension DimensionMax = INT_MAX;
static constexpr Dimension DimensionMin = INT_MIN;

template<typename T>
class Span {
public:
    T* m_Data;
    Dimension m_Size;

public:
    template<class S>
    explicit Span(S& container) : m_Data(container.data()), m_Size(container.size()) {}
};

enum class Color {
    Black,
    White,
    Red,
    Green,
    Gray,
    Blue
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
        case Color::Gray: return {127, 127, 127};
        case Color::Blue: return {0, 0, 255};
    }
}

template<class T>
class SDLHandle {
public:
    T* m_UnderlyingHandle{nullptr};

public:
    SDLHandle() = default;
    SDLHandle(T* handle) : m_UnderlyingHandle(handle) {}

    operator T*() const { return m_UnderlyingHandle; }
};

template<class T, void (*func)(T*)>
class SDLDestructor {
public:
    using pointer  = SDLHandle<T>;

public:
    void operator()(SDLHandle<T> handle) const {
        func(handle);
    }
};

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
    BlackQueen,

    AmmoPickup,
    HealthPickup
};

struct PieceMove {
    Dimension m_Dx;
    Dimension m_Dy;
    bool m_Fill;
};

static std::vector<PieceMove> EnumeratePieceMoves(Piece piece) {
    switch(piece) {
        case Piece::AmmoPickup:
        case Piece::HealthPickup:
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

class Context;

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
            std::string full_path = m_ResourceDirectory + "/" + path;
            m_Resources[m_ResourcesLast] = std::move(T(full_path, ctx));
            it->second = m_ResourcesLast++;
        }

        return m_Resources[it->second];
    }
};

class Texture;
struct TextureLoaderWrapper;

class Board {
public:
    constexpr static Dimension SquareScale = 40;
    constexpr static Dimension Width = 8;
    constexpr static Dimension Height = 8;

private:

    std::array<Piece, Width * Height> m_Board{};

    std::unordered_map<Piece, std::reference_wrapper<Texture>> m_PieceTextures;

public:
    static bool IsInBounds(Dimension x, Dimension y) {
        return !(x < 0 || y < 0 || x >= Board::Width || y >= Board::Height);
    }

    Board(TextureLoaderWrapper& loader, Context& ctx);

    void Draw(Context& ctx);

    void Set(Dimension x, Dimension y, Piece piece) {
        m_Board[x + Width * y] = piece;
    }

    Piece Get(Dimension x, Dimension y) {
        return m_Board[x + Width * y];
    }
};

class Context {
private:
    static void WindowDeleter(SDL_Window* window) { SDL_DestroyWindow(window); };
    using WindowHandle = std::unique_ptr<SDLHandle<SDL_Window>, SDLDestructor<SDL_Window, WindowDeleter>>;

    static void RendererDeleter(SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); };
    using RendererHandle = std::unique_ptr<SDLHandle<SDL_Renderer>, SDLDestructor<SDL_Renderer, RendererDeleter>>;

    static std::random_device RNG;

    static constexpr const char Title[] = "Chess with Guns";
public:
    static constexpr Dimension SidebarWidth = 192;
    static constexpr Dimension Width = Board::Width * Board::SquareScale + SidebarWidth;
    static constexpr Dimension Height = Board::Height * Board::SquareScale;

    Dimension m_ShakeIntensity = 0;
private:
    WindowHandle m_Window;
    RendererHandle m_Renderer;

    std::unordered_map<SDL_KeyCode, bool> m_KeyStates;
    bool m_MouseHeld{};

    friend class Texture;

public:
    Context() {
        int result = SDL_Init(SDL_INIT_EVERYTHING);
        if(result < 0) throw std::runtime_error("Could not init SDL2");

        result = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF);
        if(result < 0) throw std::runtime_error("Could not init SDL2_image");

        SDL_Window* window = SDL_CreateWindow(Title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        SDLNullCheck(window);
        m_Window.reset(window);

        SDL_Renderer* renderer = SDL_CreateRenderer(m_Window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        SDLNullCheck(renderer);
        m_Renderer.reset(renderer);
    }

    ~Context() {
        SDL_Quit();
    }

    static float SignedRandRange(float range) {
        return ((static_cast<float>(RNG()) / static_cast<float>(std::random_device::max())) * (2 * range)) - range;
    }

    static Dimension SignedRandRange(Dimension range) {
        return static_cast<Dimension>(((static_cast<float>(RNG()) / static_cast<float>(std::random_device::max())) * static_cast<float>(2 * range)) - static_cast<float>(range));
    }

    static Dimension UnsignedRandRange(Dimension range) {
        float mul = static_cast<float>(RNG()) / static_cast<float>(std::random_device::max());
        return static_cast<Dimension>(mul * static_cast<float>(range));
    }

    bool Update() {
        if(m_ShakeIntensity) m_ShakeIntensity -= UnsignedRandRange(2);
        if(m_ShakeIntensity <= 0) m_ShakeIntensity = 0;

        SDL_RenderPresent(m_Renderer.get());

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
        SDLResultCheck(SDL_SetRenderDrawColor(m_Renderer.get(), sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    }

    void Clear(Color color) {
        SetColor(color);
        SDLResultCheck(SDL_RenderClear(m_Renderer.get()));
    }

    void DrawRect(Dimension x, Dimension y, Dimension w, Dimension h, Color color) {
        SetColor(color);

        SDL_Rect rect {x + SignedRandRange(m_ShakeIntensity), y + SignedRandRange(m_ShakeIntensity), w, h};
        SDLResultCheck(SDL_RenderFillRect(m_Renderer.get(), &rect));
    }

    [[nodiscard]] bool IsMouseHeld() const {
        return m_MouseHeld;
    }

    [[nodiscard]] bool WasMousePressed() {
        bool pressed = IsMouseHeld();
        m_MouseHeld = false;
        return pressed;
    }

    [[nodiscard]] static std::pair<Dimension, Dimension> GetMousePosition() {
        std::pair<Dimension, Dimension> ret;
        SDL_GetMouseState(&ret.first, &ret.second);
        return ret;
    }

    template<class T>
    using DialogChoices = std::vector<std::pair<T, std::string>>;

    template<class T>
    static T ChoiceDialog(DialogChoices<T> options, const std::string& title, const std::string& message) {
        auto* buttons = new SDL_MessageBoxButtonData[options.size()];
        for(Dimension i = 0; i < options.size(); ++i) {
            buttons[i] = SDL_MessageBoxButtonData{ 0, (int) options[i].first, options[i].second.c_str() };
        }

        SDL_MessageBoxData data{ SDL_MESSAGEBOX_INFORMATION, nullptr, title.c_str(), message.c_str(), static_cast<int>(options.size()), buttons, nullptr };
        T result;
        SDLResultCheck(SDL_ShowMessageBox(&data, (int*) &result));

        delete[] buttons;
        return result;
    }

    static bool ChoiceDialog(const std::string& title, const std::string& message) {
        SDL_MessageBoxButtonData buttons[] = {
            {0, 0, "No"},
            {0, 1, "Yes"}
        };
        SDL_MessageBoxData data{ SDL_MESSAGEBOX_INFORMATION, nullptr, title.c_str(), message.c_str(), sizeof(buttons) / sizeof(buttons[0]), buttons, nullptr };
        int result;
        SDLResultCheck(SDL_ShowMessageBox(&data, (int*) &result));
        return result;
    }

    static void Dialog(const std::string& title, const std::string& message) {
        SDLResultCheck(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr));
    }
};

std::random_device Context::RNG{};

class Texture {
private:
    static void Deleter(SDL_Texture* texture) { SDL_DestroyTexture(texture); };
    using Handle = std::unique_ptr<SDLHandle<SDL_Texture>, SDLDestructor<SDL_Texture, Deleter>>;

private:
    Handle m_Texture{};
    Dimension m_Width{};
    Dimension m_Height{};

    bool m_Dummy;
public:
    static Texture Dummy;

public:
    Texture() : m_Dummy(true) {};

    Texture(const std::string& path, Context& ctx) : m_Dummy(false) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        SDLNullCheck(surface);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ctx.m_Renderer.get(), surface);
        SDLNullCheck(texture);
        m_Texture.reset(texture);

        m_Width = surface->w;
        m_Height = surface->h;

        SDL_FreeSurface(surface);
    }

    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height) {
        if(!m_Dummy) {
            SDL_Rect src {0, 0, m_Width, m_Height};
            SDL_Rect dest {x + Context::SignedRandRange(ctx.m_ShakeIntensity), y + Context::SignedRandRange(ctx.m_ShakeIntensity), width, height};
            SDLResultCheck(SDL_RenderCopy(ctx.m_Renderer.get(), m_Texture.get(), &src, &dest));
        }
    }

    void Draw(Context& ctx, Dimension x, Dimension y, Dimension width, Dimension height, float rotation) {
        if(!m_Dummy) {
            SDL_Rect src {0, 0, m_Width, m_Height};
            SDL_Rect dest {x + Context::SignedRandRange(ctx.m_ShakeIntensity), y + Context::SignedRandRange(ctx.m_ShakeIntensity), width, height};
            SDLResultCheck(SDL_RenderCopyEx(ctx.m_Renderer.get(), m_Texture.get(), &src, &dest, rotation, nullptr, SDL_FLIP_NONE));
        }
    }
};

Texture Texture::Dummy{};

using TextureLoader = ResourceLoader<Texture, 1024>;

struct TextureLoaderWrapper {
    TextureLoader m_Loader;

    explicit TextureLoaderWrapper(TextureLoader loader) : m_Loader(std::move(loader)) {}

    Texture& Get(const std::string& path, Context& ctx) { return m_Loader.Get(path, ctx); }
};

Board::Board(TextureLoaderWrapper& loader, Context& ctx) : m_Board{}, m_PieceTextures{} {
    m_PieceTextures.insert({Piece::None, Texture::Dummy});

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

    m_PieceTextures.insert({Piece::AmmoPickup, loader.Get("AmmoPickup.png", ctx)});
    m_PieceTextures.insert({Piece::HealthPickup, loader.Get("HealthPickup.png", ctx)});
}


void Board::Draw(Context& ctx) {
    for(Dimension i = 0; i < Height; ++i) {
        for(Dimension j = 0; j < Width; ++j) {
            ctx.DrawRect(j * SquareScale, i * SquareScale, SquareScale, SquareScale, (j + i % 2) % 2 ? Color::Black : Color::White);
            m_PieceTextures.at(m_Board[j + i * Width]).get().Draw(ctx, j * SquareScale, i * SquareScale, SquareScale, SquareScale);
        }
    }
}

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

            auto x = static_cast<Dimension>(m_X);
            auto y = static_cast<Dimension>(m_Y);

            ctx.DrawRect(x, y, ProjectileScale, ProjectileScale, Color::Red);

            if(!Board::IsInBounds(x / Board::SquareScale, y / Board::SquareScale)) {
                m_Shown = false;
                return Piece::None;
            }

            Piece piece = board.Get(x / Board::SquareScale, y / Board::SquareScale);
            if(piece != Piece::None && piece != ignore && piece != Piece::AmmoPickup && piece != Piece::HealthPickup) {
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
                if(at == Piece::None || at == Piece::AmmoPickup || at == Piece::HealthPickup) {
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
                    if(at == Piece::AmmoPickup || at == Piece::HealthPickup) {
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

        if(at == Piece::AmmoPickup || at == Piece::HealthPickup) {
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
                if(at == Piece::AmmoPickup || at == Piece::HealthPickup) {
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
                Piece hit = projectile.DoMove(ctx, board, fired.m_Piece);
                if(hit != Piece::None) {
                    projectile.m_Shown = false;
                    float damage = WeaponDamage(fired.m_Weapon) + Context::SignedRandRange(WeaponVariance(fired.m_Weapon));
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
