#pragma once
#include <vector>
#include "Position.h"

class Board; // forward declaration (avoids circular include with Board.h)

enum class Color { WHITE, BLACK };

// ---------- Abstract base class ----------
class Piece {
protected:
    Color color;

    // Walks from 'from' in direction (dRow, dCol) until blocked.
    // Shared by Rook, Bishop and Queen.
    void addSlidingMoves(const Board& board, Position from, int dRow, int dCol,
                         std::vector<Position>& moves) const;

public:
    Piece(Color c) : color(c) {}
    virtual ~Piece() = default;

    Color getColor() const { return color; }

    virtual std::vector<Position> getValidMoves(const Board& board, Position current) const = 0;
    virtual char getSymbol() const = 0;
};

// ---------- Derived pieces ----------
class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'P' : 'p'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};

class Knight : public Piece {
public:
    Knight(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'N' : 'n'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};

class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'B' : 'b'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};

class Rook : public Piece {
public:
    Rook(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'R' : 'r'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};

class Queen : public Piece {
public:
    Queen(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'Q' : 'q'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};

class King : public Piece {
public:
    King(Color c) : Piece(c) {}
    char getSymbol() const override { return color == Color::WHITE ? 'K' : 'k'; }
    std::vector<Position> getValidMoves(const Board& board, Position current) const override;
};