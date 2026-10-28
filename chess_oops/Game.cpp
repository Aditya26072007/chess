#include "Game.h"
#include <algorithm>

Game::Game() : currentTurn(Color::WHITE), state(GameState::ONGOING) {
    board.setupBoard();
}

// Is 'color' king attacked by any enemy piece?
bool Game::isInCheck(Color color) const {
    Position king = board.findKing(color);
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Position from{r, c};
            Piece* p = board.getPieceAt(from);
            if (p == nullptr || p->getColor() == color) continue;
            for (const Position& target : p->getValidMoves(board, from))
                if (target == king) return true;
        }
    return false;
}

// Simulate the move, check if the mover's king is attacked, then undo it.
bool Game::wouldLeaveKingInCheck(Position from, Position to) {
    Color mover = board.getPieceAt(from)->getColor();
    std::unique_ptr<Piece> captured = board.movePiece(from, to);   // simulate
    bool inCheck = isInCheck(mover);                               // test
    board.undoMove(from, to, std::move(captured));                 // undo
    return inCheck;
}

// Pseudo-legal moves minus the ones that expose your own king.
std::vector<Position> Game::getLegalMoves(Position from) {
    std::vector<Position> legal;
    Piece* p = board.getPieceAt(from);
    if (p == nullptr) return legal;
    for (const Position& to : p->getValidMoves(board, from))
        if (!wouldLeaveKingInCheck(from, to))
            legal.push_back(to);
    return legal;
}

bool Game::hasAnyLegalMove(Color color) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Position from{r, c};
            Piece* p = board.getPieceAt(from);
            if (p && p->getColor() == color && !getLegalMoves(from).empty())
                return true;
        }
    return false;
}

bool Game::isCheckmate(Color color) {
    return isInCheck(color) && !hasAnyLegalMove(color);
}

bool Game::makeMove(Position from, Position to) {
    if (state == GameState::CHECKMATE) return false;

    // 1. Must be your own piece
    Piece* p = board.getPieceAt(from);
    if (p == nullptr || p->getColor() != currentTurn) return false;

    // 2. Destination must be a legal move
    std::vector<Position> legal = getLegalMoves(from);
    if (std::find(legal.begin(), legal.end(), to) == legal.end()) return false;

    // 3. Execute and switch turns
    board.movePiece(from, to);
    currentTurn = (currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // 4. Update game state for the player who now has to move
    if (isCheckmate(currentTurn))     state = GameState::CHECKMATE;
    else if (isInCheck(currentTurn))  state = GameState::CHECK;
    else                              state = GameState::ONGOING;
    return true;
}