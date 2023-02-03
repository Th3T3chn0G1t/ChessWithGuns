#include <CWG.hpp>
#include <FX.hpp>
#include <Texture.hpp>
#include <Context.hpp>

bool Board::IsInBounds(Dimension x, Dimension y) {
    return !(x < 0 || y < 0 || x >= Board::Width || y >= Board::Height);
}

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
    m_PieceTextures.insert({Piece::BoostPickup, loader.Get("BoostPickup.png", ctx)});
}

void Board::Draw(Context& ctx) {
    for(Dimension i = 0; i < Height; ++i) {
        for(Dimension j = 0; j < Width; ++j) {
            ctx.DrawRect(j * SquareScale, i * SquareScale, SquareScale, SquareScale, (j + i % 2) % 2 ? Color::Black : Color::White);
            m_PieceTextures.at(m_Board[j + i * Width]).get().Draw(ctx, j * SquareScale, i * SquareScale, SquareScale, SquareScale);
        }
    }
}

void Board::Set(Dimension x, Dimension y, Piece piece) {
    m_Board[x + Width * y] = piece;
}

Piece Board::Get(Dimension x, Dimension y) {
    return m_Board[x + Width * y];
}
