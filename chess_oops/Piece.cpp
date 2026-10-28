#include "Piece.h"
#include "Board.h"

// ---------- Shared helper for Rook, Bishop, Queen ----------
void Piece::addSlidingMoves(const Board& board, Position from, int dRow, int dCol,
                            std::vector<Position>& moves) const {
    Position p{from.row + dRow, from.col + dCol};
    while (p.isValid()) {
        if (!board.isSquareOccupied(p)) {
            moves.push_back(p);                 // empty: keep sliding
        } else {
            if (board.isEnemyPiece(p, color))   // enemy: can capture, then stop
                moves.push_back(p);
            break;                              // any piece blocks the path
        }
        p.row += dRow;
        p.col += dCol;
    }
}

// ---------- Knight ----------
std::vector<Position> Knight::getValidMoves(const Board& board, Position current) const {
    static const int offsets[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    std::vector<Position> moves;
    for (const auto& o : offsets) {
        Position p{current.row + o[0], current.col + o[1]};
        if (!p.isValid()) continue;
        Piece* target = board.getPieceAt(p);
        if (target == nullptr || target->getColor() != color)
            moves.push_back(p);
    }
    return moves;
}

// ---------- King ----------
std::vector<Position> King::getValidMoves(const Board& board, Position current) const {
    std::vector<Position> moves;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            Position p{current.row + dr, current.col + dc};
            if (!p.isValid()) continue;
            Piece* target = board.getPieceAt(p);
            if (target == nullptr || target->getColor() != color)
                moves.push_back(p);
        }
    }
    return moves;
}

// ---------- Rook ----------
std::vector<Position> Rook::getValidMoves(const Board& board, Position current) const {
    std::vector<Position> moves;
    addSlidingMoves(board, current, -1,  0, moves);   // up
    addSlidingMoves(board, current,  1,  0, moves);   // down
    addSlidingMoves(board, current,  0, -1, moves);   // left
    addSlidingMoves(board, current,  0,  1, moves);   // right
    return moves;
}

// ---------- Bishop ----------
std::vector<Position> Bishop::getValidMoves(const Board& board, Position current) const {
    std::vector<Position> moves;
    addSlidingMoves(board, current, -1, -1, moves);   // up-left
    addSlidingMoves(board, current, -1,  1, moves);   // up-right
    addSlidingMoves(board, current,  1, -1, moves);   // down-left
    addSlidingMoves(board, current,  1,  1, moves);   // down-right
    return moves;
}

// ---------- Queen (rook + bishop directions) ----------
std::vector<Position> Queen::getValidMoves(const Board& board, Position current) const {
    std::vector<Position> moves;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++)
            if (dr != 0 || dc != 0)
                addSlidingMoves(board, current, dr, dc, moves);
    return moves;
}

// ---------- Pawn ----------
std::vector<Position> Pawn::getValidMoves(const Board& board, Position current) const {
    std::vector<Position> moves;
    int dir = (color == Color::WHITE) ? -1 : 1;        // white moves toward row 0
    int startRow = (color == Color::WHITE) ? 6 : 1;

    // Forward one (only if empty)
    Position one{current.row + dir, current.col};
    if (one.isValid() && !board.isSquareOccupied(one)) {
        moves.push_back(one);
        // Forward two from the start row (only if both squares are empty)
        Position two{current.row + 2 * dir, current.col};
        if (current.row == startRow && !board.isSquareOccupied(two))
            moves.push_back(two);
    }

    // Diagonal captures (only onto an enemy piece)
    for (int dc : {-1, 1}) {
        Position diag{current.row + dir, current.col + dc};
        if (diag.isValid() && board.isEnemyPiece(diag, color))
            moves.push_back(diag);
    }
    return moves;
}