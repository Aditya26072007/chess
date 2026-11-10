#pragma once
#include <vector>
#include <string>
#include "Board.h"

enum class GameState { ONGOING, CHECK, CHECKMATE, STALEMATE };
enum class SpecialMove { NONE, CASTLE_KINGSIDE, CASTLE_QUEENSIDE, EN_PASSANT, PROMOTION };

// Snapshot of castling/en-passant flags so undo can restore them.
struct StateFlags {
    bool kingMoved[2]  = {false, false};   // [0]=WHITE, [1]=BLACK
    bool rookAMoved[2] = {false, false};   // a-file (queenside) rook
    bool rookHMoved[2] = {false, false};   // h-file (kingside) rook
    Position enPassantTarget{-1, -1};      // square where en-passant capture is allowed
};

// Stores everything needed to undo a single move.
struct MoveRecord {
    Position from;
    Position to;
    char movedSymbol;                       // symbol of the piece that moved
    std::unique_ptr<Piece> captured;        // nullptr if no capture (normal or castling)
    GameState prevState;
    Color prevTurn;
    std::string notation;                   // algebraic notation string
    bool wasCapture = false;                // whether this move captured a piece

    SpecialMove special = SpecialMove::NONE;

    // Castling undo info
    Position rookFrom{-1, -1};
    Position rookTo{-1, -1};

    // En passant undo info
    Position epCapturedPos{-1, -1};
    std::unique_ptr<Piece> epCapturedPiece;

    // Promotion undo info
    char promotedTo = '\0';
    std::unique_ptr<Piece> originalPawn;    // the pawn before promotion

    // Previous state flags (for undo)
    StateFlags prevFlags;
};

class Game {
private:
    Board board;
    Color currentTurn;
    GameState state;
    StateFlags flags;
    std::vector<MoveRecord> history;
    std::vector<std::pair<char, Color>> capturedPieces;

    bool wouldLeaveKingInCheck(Position from, Position to);
    bool hasAnyLegalMove(Color color);
    std::string buildNotation(Position from, Position to, char movedSym,
                               bool wasCapture, bool isCheck, bool isMate,
                               SpecialMove special, char promotedTo);

    // Internal helpers for castling validation
    int colorIdx(Color c) const { return c == Color::WHITE ? 0 : 1; }
    bool canCastleKingside(Color color) const;
    bool canCastleQueenside(Color color) const;
    bool isSquareAttacked(Position pos, Color byColor) const;

public:
    Game();
    bool makeMove(Position from, Position to, char promoteTo = 'Q');
    bool undoLastMove();
    std::vector<Position> getLegalMoves(Position from);
    bool isInCheck(Color color) const;
    bool isCheckmate(Color color);
    bool isStalemate(Color color);
    bool isPromotionMove(Position from, Position to) const;
    void reset();

    Color getCurrentTurn() const { return currentTurn; }
    GameState getState() const { return state; }
    const Board& getBoard() const { return board; }
    const std::vector<MoveRecord>& getHistory() const { return history; }
    const std::vector<std::pair<char, Color>>& getCapturedPieces() const { return capturedPieces; }
    int getMoveCount() const { return static_cast<int>(history.size()); }
};