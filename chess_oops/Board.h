#pragma once
#include <array>
#include <memory>
#include <iostream>
#include "Piece.h"

class Board {
private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid;

public:
    void setupBoard();                                   // places all 32 pieces
    Piece* getPieceAt(Position pos) const;               // nullptr if empty or off-board
    bool isSquareOccupied(Position pos) const;
    bool isEnemyPiece(Position pos, Color myColor) const;

    // Returns the captured piece (nullptr if the target square was empty).
    std::unique_ptr<Piece> movePiece(Position from, Position to);
    void undoMove(Position from, Position to, std::unique_ptr<Piece> captured);

    Position findKing(Color color) const;                // {-1,-1} if not found
    void printBoard(std::ostream& os = std::cout) const;
};

std::ostream& operator<<(std::ostream& os, const Board& board);