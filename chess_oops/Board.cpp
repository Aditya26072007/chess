#include "Board.h"

// Builds the back-rank piece for a given column (0 = a-file ... 7 = h-file).
static std::unique_ptr<Piece> makeBackRankPiece(int col, Color c) {
    switch (col) {
        case 0: case 7: return std::make_unique<Rook>(c);
        case 1: case 6: return std::make_unique<Knight>(c);
        case 2: case 5: return std::make_unique<Bishop>(c);
        case 3:         return std::make_unique<Queen>(c);
        default:        return std::make_unique<King>(c);   // col 4
    }
}

void Board::setupBoard() {
    // Clear everything first
    for (auto& row : grid)
        for (auto& square : row)
            square.reset();

    // Row 0 = rank 8 (black), row 7 = rank 1 (white)
    for (int col = 0; col < 8; col++) {
        grid[0][col] = makeBackRankPiece(col, Color::BLACK);
        grid[1][col] = std::make_unique<Pawn>(Color::BLACK);
        grid[6][col] = std::make_unique<Pawn>(Color::WHITE);
        grid[7][col] = makeBackRankPiece(col, Color::WHITE);
    }
}

Piece* Board::getPieceAt(Position pos) const {
    if (!pos.isValid()) return nullptr;
    return grid[pos.row][pos.col].get();
}

bool Board::isSquareOccupied(Position pos) const {
    return getPieceAt(pos) != nullptr;
}

bool Board::isEnemyPiece(Position pos, Color myColor) const {
    Piece* p = getPieceAt(pos);
    return p != nullptr && p->getColor() != myColor;
}

std::unique_ptr<Piece> Board::movePiece(Position from, Position to) {
    std::unique_ptr<Piece> captured = std::move(grid[to.row][to.col]);
    grid[to.row][to.col] = std::move(grid[from.row][from.col]);
    return captured;
}

void Board::undoMove(Position from, Position to, std::unique_ptr<Piece> captured) {
    grid[from.row][from.col] = std::move(grid[to.row][to.col]);
    grid[to.row][to.col] = std::move(captured);
}

Position Board::findKing(Color color) const {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece* p = grid[r][c].get();
            if (p && p->getColor() == color && dynamic_cast<King*>(p))
                return Position{r, c};
        }
    return Position{-1, -1};
}

void Board::printBoard(std::ostream& os) const {
    os << std::endl << "    a b c d e f g h" << std::endl;
    for (int r = 0; r < 8; r++) {
        os << " " << 8 - r << "  ";
        for (int c = 0; c < 8; c++) {
            Piece* p = grid[r][c].get();
            os << (p ? p->getSymbol() : '.') << ' ';
        }
        os << " " << 8 - r << std::endl;
    }
    os << "    a b c d e f g h" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const Board& board) {
    board.printBoard(os);
    return os;
}