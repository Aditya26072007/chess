#pragma once
#include <vector>
#include "Board.h"

enum class GameState { ONGOING, CHECK, CHECKMATE };

class Game {
private:
    Board board;
    Color currentTurn;
    GameState state;

    bool wouldLeaveKingInCheck(Position from, Position to);   // simulate, test, undo
    bool hasAnyLegalMove(Color color);

public:
    Game();
    bool makeMove(Position from, Position to);                // validates, then executes
    std::vector<Position> getLegalMoves(Position from);       // moves that don't expose own king
    bool isInCheck(Color color) const;
    bool isCheckmate(Color color);

    Color getCurrentTurn() const { return currentTurn; }
    GameState getState() const { return state; }
    const Board& getBoard() const { return board; }
};