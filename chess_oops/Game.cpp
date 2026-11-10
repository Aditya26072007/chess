#include "Game.h"
#include <algorithm>
#include <cctype>

Game::Game() : currentTurn(Color::WHITE), state(GameState::ONGOING) {
    board.setupBoard();
}

void Game::reset() {
    board.setupBoard();
    currentTurn = Color::WHITE;
    state = GameState::ONGOING;
    flags = StateFlags{};
    history.clear();
    capturedPieces.clear();
}

// ─────────────────────────────────────────────
//   CHECK / ATTACK DETECTION
// ─────────────────────────────────────────────

// Is 'pos' attacked by any piece of color 'byColor'?
bool Game::isSquareAttacked(Position pos, Color byColor) const {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Position from{r, c};
            Piece* p = board.getPieceAt(from);
            if (p == nullptr || p->getColor() != byColor) continue;
            for (const Position& target : p->getValidMoves(board, from))
                if (target == pos) return true;
        }
    return false;
}

// Is 'color' king attacked by any enemy piece?
bool Game::isInCheck(Color color) const {
    Position king = board.findKing(color);
    Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    return isSquareAttacked(king, enemy);
}

// Simulate the move, check if the mover's king is attacked, then undo it.
bool Game::wouldLeaveKingInCheck(Position from, Position to) {
    Color mover = board.getPieceAt(from)->getColor();
    std::unique_ptr<Piece> captured = board.movePiece(from, to);
    bool inCheck = isInCheck(mover);
    board.undoMove(from, to, std::move(captured));
    return inCheck;
}

// ─────────────────────────────────────────────
//   CASTLING VALIDATION
// ─────────────────────────────────────────────

bool Game::canCastleKingside(Color color) const {
    int ci = colorIdx(color);
    if (flags.kingMoved[ci] || flags.rookHMoved[ci]) return false;

    int row = (color == Color::WHITE) ? 7 : 0;
    Position kingPos{row, 4};
    Position rookPos{row, 7};

    // King must be at e1/e8 and rook at h1/h8
    Piece* king = board.getPieceAt(kingPos);
    Piece* rook = board.getPieceAt(rookPos);
    if (!king || !rook) return false;
    if (dynamic_cast<King*>(king) == nullptr) return false;
    if (dynamic_cast<Rook*>(rook) == nullptr) return false;

    // Squares between must be empty (f and g)
    if (board.isSquareOccupied(Position{row, 5})) return false;
    if (board.isSquareOccupied(Position{row, 6})) return false;

    // King must not be in check
    Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (isSquareAttacked(kingPos, enemy)) return false;

    // King must not pass through or land on attacked squares
    if (isSquareAttacked(Position{row, 5}, enemy)) return false;
    if (isSquareAttacked(Position{row, 6}, enemy)) return false;

    return true;
}

bool Game::canCastleQueenside(Color color) const {
    int ci = colorIdx(color);
    if (flags.kingMoved[ci] || flags.rookAMoved[ci]) return false;

    int row = (color == Color::WHITE) ? 7 : 0;
    Position kingPos{row, 4};
    Position rookPos{row, 0};

    Piece* king = board.getPieceAt(kingPos);
    Piece* rook = board.getPieceAt(rookPos);
    if (!king || !rook) return false;
    if (dynamic_cast<King*>(king) == nullptr) return false;
    if (dynamic_cast<Rook*>(rook) == nullptr) return false;

    // Squares between must be empty (b, c, d)
    if (board.isSquareOccupied(Position{row, 1})) return false;
    if (board.isSquareOccupied(Position{row, 2})) return false;
    if (board.isSquareOccupied(Position{row, 3})) return false;

    // King must not be in check
    Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (isSquareAttacked(kingPos, enemy)) return false;

    // King must not pass through or land on attacked squares (d and c)
    if (isSquareAttacked(Position{row, 3}, enemy)) return false;
    if (isSquareAttacked(Position{row, 2}, enemy)) return false;

    return true;
}

// ─────────────────────────────────────────────
//   LEGAL MOVE GENERATION
// ─────────────────────────────────────────────

std::vector<Position> Game::getLegalMoves(Position from) {
    std::vector<Position> legal;
    Piece* p = board.getPieceAt(from);
    if (p == nullptr) return legal;

    Color color = p->getColor();

    // Basic pseudo-legal moves filtered for check
    for (const Position& to : p->getValidMoves(board, from))
        if (!wouldLeaveKingInCheck(from, to))
            legal.push_back(to);

    // ─── Castling (King only) ───
    if (dynamic_cast<King*>(p) && p->getColor() == currentTurn) {
        int row = (color == Color::WHITE) ? 7 : 0;
        if (from.row == row && from.col == 4) {
            if (canCastleKingside(color))
                legal.push_back(Position{row, 6});
            if (canCastleQueenside(color))
                legal.push_back(Position{row, 2});
        }
    }

    // ─── En Passant (Pawn only) ───
    if (dynamic_cast<Pawn*>(p) && flags.enPassantTarget.isValid()) {
        int dir = (color == Color::WHITE) ? -1 : 1;
        // The pawn can capture en passant if it's adjacent to the target column
        // and on the correct row, and the target is the en passant square
        Position epTarget = flags.enPassantTarget;
        if (from.row + dir == epTarget.row && std::abs(from.col - epTarget.col) == 1) {
            // Simulate the en passant to make sure it doesn't leave king in check
            Position capturedPawnPos{from.row, epTarget.col};
            std::unique_ptr<Piece> movedPiece = board.movePiece(from, epTarget);
            std::unique_ptr<Piece> capturedPawn = board.removePiece(capturedPawnPos);
            bool inCheck = isInCheck(color);
            // Undo
            board.placePiece(capturedPawnPos, std::move(capturedPawn));
            board.undoMove(from, epTarget, std::move(movedPiece));
            if (!inCheck)
                legal.push_back(epTarget);
        }
    }

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

bool Game::isStalemate(Color color) {
    return !isInCheck(color) && !hasAnyLegalMove(color);
}

// ─────────────────────────────────────────────
//   PROMOTION CHECK
// ─────────────────────────────────────────────

bool Game::isPromotionMove(Position from, Position to) const {
    Piece* p = board.getPieceAt(from);
    if (!p || dynamic_cast<Pawn*>(p) == nullptr) return false;
    int promoRow = (p->getColor() == Color::WHITE) ? 0 : 7;
    return to.row == promoRow;
}

// ─────────────────────────────────────────────
//   NOTATION
// ─────────────────────────────────────────────

std::string Game::buildNotation(Position from, Position to, char movedSym,
                                 bool wasCapture, bool isCheck, bool isMate,
                                 SpecialMove special, char promotedTo) {
    if (special == SpecialMove::CASTLE_KINGSIDE)  { std::string n = "O-O";   if (isMate) n += '#'; else if (isCheck) n += '+'; return n; }
    if (special == SpecialMove::CASTLE_QUEENSIDE) { std::string n = "O-O-O"; if (isMate) n += '#'; else if (isCheck) n += '+'; return n; }

    std::string note;
    char upperSym = static_cast<char>(std::toupper(static_cast<unsigned char>(movedSym)));
    char toFile = static_cast<char>('a' + to.col);
    char toRank = static_cast<char>('0' + (8 - to.row));

    if (upperSym == 'P') {
        if (wasCapture) {
            note += static_cast<char>('a' + from.col);
            note += 'x';
        }
        note += toFile;
        note += toRank;
        if (special == SpecialMove::PROMOTION) {
            note += '=';
            note += static_cast<char>(std::toupper(static_cast<unsigned char>(promotedTo)));
        }
    } else {
        note += upperSym;
        if (wasCapture) note += 'x';
        note += toFile;
        note += toRank;
    }

    if (isMate)      note += '#';
    else if (isCheck) note += '+';

    return note;
}

// ─────────────────────────────────────────────
//   MAKE MOVE
// ─────────────────────────────────────────────

bool Game::makeMove(Position from, Position to, char promoteTo) {
    if (state == GameState::CHECKMATE || state == GameState::STALEMATE) return false;

    Piece* p = board.getPieceAt(from);
    if (p == nullptr || p->getColor() != currentTurn) return false;

    // Check legality
    std::vector<Position> legal = getLegalMoves(from);
    if (std::find(legal.begin(), legal.end(), to) == legal.end()) return false;

    // ─── Build the move record ───
    MoveRecord record;
    record.from = from;
    record.to = to;
    record.movedSymbol = p->getSymbol();
    record.prevState = state;
    record.prevTurn = currentTurn;
    record.prevFlags = flags;

    int ci = colorIdx(currentTurn);
    Color color = currentTurn;
    bool wasCapture = false;

    // ─── Detect special move type ───

    // Castling?
    if (dynamic_cast<King*>(p)) {
        int row = (color == Color::WHITE) ? 7 : 0;
        if (from.row == row && from.col == 4 && to.col == 6) {
            record.special = SpecialMove::CASTLE_KINGSIDE;
        } else if (from.row == row && from.col == 4 && to.col == 2) {
            record.special = SpecialMove::CASTLE_QUEENSIDE;
        }
    }

    // En passant?
    if (dynamic_cast<Pawn*>(p) && to == flags.enPassantTarget) {
        record.special = SpecialMove::EN_PASSANT;
    }

    // Promotion?
    if (dynamic_cast<Pawn*>(p)) {
        int promoRow = (color == Color::WHITE) ? 0 : 7;
        if (to.row == promoRow) {
            record.special = SpecialMove::PROMOTION;
            record.promotedTo = promoteTo;
        }
    }

    // ─── Execute the move ───

    if (record.special == SpecialMove::CASTLE_KINGSIDE) {
        int row = from.row;
        record.rookFrom = Position{row, 7};
        record.rookTo = Position{row, 5};
        board.movePiece(from, to);                            // move king
        board.movePiece(record.rookFrom, record.rookTo);      // move rook
        flags.kingMoved[ci] = true;
        flags.rookHMoved[ci] = true;

    } else if (record.special == SpecialMove::CASTLE_QUEENSIDE) {
        int row = from.row;
        record.rookFrom = Position{row, 0};
        record.rookTo = Position{row, 3};
        board.movePiece(from, to);
        board.movePiece(record.rookFrom, record.rookTo);
        flags.kingMoved[ci] = true;
        flags.rookAMoved[ci] = true;

    } else if (record.special == SpecialMove::EN_PASSANT) {
        record.epCapturedPos = Position{from.row, to.col};
        record.epCapturedPiece = board.removePiece(record.epCapturedPos);
        record.captured = board.movePiece(from, to);  // will be nullptr (target square empty)
        wasCapture = true;

        // Track captured piece
        if (record.epCapturedPiece) {
            capturedPieces.push_back({record.epCapturedPiece->getSymbol(),
                                       record.epCapturedPiece->getColor()});
        }

    } else if (record.special == SpecialMove::PROMOTION) {
        // Normal capture if target occupied
        Piece* capturedPiece = board.getPieceAt(to);
        if (capturedPiece) {
            wasCapture = true;
            capturedPieces.push_back({capturedPiece->getSymbol(), capturedPiece->getColor()});
        }
        record.captured = board.movePiece(from, to);

        // Replace pawn with promoted piece
        record.originalPawn = board.removePiece(to);
        char upperPromo = static_cast<char>(std::toupper(static_cast<unsigned char>(promoteTo)));
        std::unique_ptr<Piece> promoted;
        switch (upperPromo) {
            case 'R': promoted = std::make_unique<Rook>(color);   break;
            case 'B': promoted = std::make_unique<Bishop>(color); break;
            case 'N': promoted = std::make_unique<Knight>(color); break;
            default:  promoted = std::make_unique<Queen>(color);  break;
        }
        board.placePiece(to, std::move(promoted));

    } else {
        // Normal move
        Piece* capturedPiece = board.getPieceAt(to);
        if (capturedPiece) {
            wasCapture = true;
            capturedPieces.push_back({capturedPiece->getSymbol(), capturedPiece->getColor()});
        }
        record.captured = board.movePiece(from, to);
    }

    // ─── Update castling flags for normal king/rook moves ───
    if (record.special == SpecialMove::NONE || record.special == SpecialMove::PROMOTION
        || record.special == SpecialMove::EN_PASSANT) {
        char upperSym = static_cast<char>(std::toupper(static_cast<unsigned char>(record.movedSymbol)));
        if (upperSym == 'K') {
            flags.kingMoved[ci] = true;
        } else if (upperSym == 'R') {
            int homeRow = (color == Color::WHITE) ? 7 : 0;
            if (from.row == homeRow && from.col == 0) flags.rookAMoved[ci] = true;
            if (from.row == homeRow && from.col == 7) flags.rookHMoved[ci] = true;
        }
        // If a rook is captured, mark it as moved
        if (wasCapture && record.special != SpecialMove::EN_PASSANT) {
            int enemyCi = 1 - ci;
            int enemyHomeRow = (color == Color::WHITE) ? 0 : 7;
            if (to.row == enemyHomeRow && to.col == 0) flags.rookAMoved[enemyCi] = true;
            if (to.row == enemyHomeRow && to.col == 7) flags.rookHMoved[enemyCi] = true;
        }
    }

    // ─── Update en passant target ───
    flags.enPassantTarget = Position{-1, -1};
    if (dynamic_cast<Pawn*>(board.getPieceAt(to)) ||
        (record.special == SpecialMove::PROMOTION)) {
        // Only the pawn that just moved 2 squares sets the ep target
    }
    // Check if pawn moved two squares forward
    char upperMoved = static_cast<char>(std::toupper(static_cast<unsigned char>(record.movedSymbol)));
    if (upperMoved == 'P' && std::abs(to.row - from.row) == 2) {
        flags.enPassantTarget = Position{(from.row + to.row) / 2, from.col};
    }

    // ─── Switch turns ───
    currentTurn = (currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // ─── Update game state ───
    bool check = isInCheck(currentTurn);
    bool noMoves = !hasAnyLegalMove(currentTurn);

    if (check && noMoves)       state = GameState::CHECKMATE;
    else if (!check && noMoves) state = GameState::STALEMATE;
    else if (check)             state = GameState::CHECK;
    else                        state = GameState::ONGOING;

    // ─── Build notation ───
    record.notation = buildNotation(record.from, record.to, record.movedSymbol,
                                     wasCapture,
                                     state == GameState::CHECK,
                                     state == GameState::CHECKMATE,
                                     record.special, record.promotedTo);

    record.wasCapture = wasCapture;
    history.push_back(std::move(record));
    return true;
}

// ─────────────────────────────────────────────
//   UNDO
// ─────────────────────────────────────────────

bool Game::undoLastMove() {
    if (history.empty()) return false;

    MoveRecord& record = history.back();

    // Pop captured piece from tracking list if this move captured something
    if (record.wasCapture && !capturedPieces.empty()) {
        capturedPieces.pop_back();
    }

    if (record.special == SpecialMove::CASTLE_KINGSIDE || record.special == SpecialMove::CASTLE_QUEENSIDE) {
        // Undo rook move first, then king
        board.undoMove(record.rookFrom, record.rookTo, nullptr);
        board.undoMove(record.from, record.to, nullptr);

    } else if (record.special == SpecialMove::EN_PASSANT) {
        // Undo the pawn move
        board.undoMove(record.from, record.to, std::move(record.captured));
        // Restore the captured pawn at its original position
        board.placePiece(record.epCapturedPos, std::move(record.epCapturedPiece));

    } else if (record.special == SpecialMove::PROMOTION) {
        // Remove promoted piece from the board
        board.removePiece(record.to);
        // Put the original pawn back at 'to'
        board.placePiece(record.to, std::move(record.originalPawn));
        // Undo the move (pawn goes back to 'from', captured piece restored at 'to')
        board.undoMove(record.from, record.to, std::move(record.captured));

    } else {
        // Normal move undo
        board.undoMove(record.from, record.to, std::move(record.captured));
    }

    // Restore flags and state
    flags = record.prevFlags;
    state = record.prevState;
    currentTurn = record.prevTurn;

    history.pop_back();
    return true;
}