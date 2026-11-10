#include "chessboardwidget.h"
#include "promotiondialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QEasingCurve>
#include <QFont>
#include <cmath>

ChessBoardWidget::ChessBoardWidget(Game* game, QWidget* parent)
    : QWidget(parent), m_game(game)
{
    setMinimumSize(320, 320);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

int ChessBoardWidget::squareSize() const {
    return std::min(width(), height()) / 8;
}

QRect ChessBoardWidget::squareRect(int row, int col) const {
    int sz = squareSize();
    int boardWidth = sz * 8;
    int boardHeight = sz * 8;
    int offsetX = (width() - boardWidth) / 2;
    int offsetY = (height() - boardHeight) / 2;
    int dr = displayRow(row);
    int dc = displayCol(col);
    return QRect(offsetX + dc * sz, offsetY + dr * sz, sz, sz);
}

Position ChessBoardWidget::boardPosFromPixel(const QPoint& pixel) const {
    int sz = squareSize();
    int boardWidth = sz * 8;
    int boardHeight = sz * 8;
    int offsetX = (width() - boardWidth) / 2;
    int offsetY = (height() - boardHeight) / 2;

    int dc = (pixel.x() - offsetX) / sz;
    int dr = (pixel.y() - offsetY) / sz;

    if (dc < 0 || dc >= 8 || dr < 0 || dr >= 8)
        return Position{-1, -1};

    int row = m_flipped ? (7 - dr) : dr;
    int col = m_flipped ? (7 - dc) : dc;
    return Position{row, col};
}

QString ChessBoardWidget::pieceUnicode(char symbol) const {
    switch (symbol) {
        case 'K': return QString::fromUtf8("\u2654");  // ♔ White King
        case 'Q': return QString::fromUtf8("\u2655");  // ♕ White Queen
        case 'R': return QString::fromUtf8("\u2656");  // ♖ White Rook
        case 'B': return QString::fromUtf8("\u2657");  // ♗ White Bishop
        case 'N': return QString::fromUtf8("\u2658");  // ♘ White Knight
        case 'P': return QString::fromUtf8("\u2659");  // ♙ White Pawn
        case 'k': return QString::fromUtf8("\u265A");  // ♚ Black King
        case 'q': return QString::fromUtf8("\u265B");  // ♛ Black Queen
        case 'r': return QString::fromUtf8("\u265C");  // ♜ Black Rook
        case 'b': return QString::fromUtf8("\u265D");  // ♝ Black Bishop
        case 'n': return QString::fromUtf8("\u265E");  // ♞ Black Knight
        case 'p': return QString::fromUtf8("\u265F");  // ♟ Black Pawn
        default:  return QString();
    }
}

void ChessBoardWidget::clearSelection() {
    m_selected = Position{-1, -1};
    m_legalMoves.clear();
}

void ChessBoardWidget::updateKingInCheck() {
    if (m_game->getState() == GameState::CHECK || m_game->getState() == GameState::CHECKMATE) {
        m_kingInCheck = m_game->getBoard().findKing(m_game->getCurrentTurn());
    } else {
        m_kingInCheck = Position{-1, -1};
    }
}

void ChessBoardWidget::refreshBoard() {
    updateKingInCheck();
    clearSelection();
    update();
}

void ChessBoardWidget::setFlipped(bool flipped) {
    m_flipped = flipped;
    update();
}

void ChessBoardWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}

// ─────────────────────────────────────────────
//   PAINT
// ─────────────────────────────────────────────
void ChessBoardWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    int sz = squareSize();
    int boardWidth = sz * 8;
    int boardHeight = sz * 8;
    int offsetX = (width() - boardWidth) / 2;
    int offsetY = (height() - boardHeight) / 2;

    // Draw board shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.drawRoundedRect(offsetX + 4, offsetY + 4, boardWidth, boardHeight, 2, 2);

    // Draw squares
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            QRect rect = squareRect(r, c);
            bool isLight = (r + c) % 2 == 0;
            painter.fillRect(rect, isLight ? lightSquare() : darkSquare());
        }
    }

    // Last-move highlight
    if (m_lastFrom.isValid()) {
        painter.fillRect(squareRect(m_lastFrom.row, m_lastFrom.col), lastMoveColor());
    }
    if (m_lastTo.isValid()) {
        painter.fillRect(squareRect(m_lastTo.row, m_lastTo.col), lastMoveColor());
    }

    // Selected square highlight
    if (m_selected.isValid()) {
        painter.fillRect(squareRect(m_selected.row, m_selected.col), selectedColor());
    }

    // King in check highlight — radial gradient glow
    if (m_kingInCheck.isValid()) {
        QRect kr = squareRect(m_kingInCheck.row, m_kingInCheck.col);
        QRadialGradient gradient(kr.center(), sz * 0.7);
        gradient.setColorAt(0, QColor(255, 0, 0, 200));
        gradient.setColorAt(0.5, QColor(255, 0, 0, 100));
        gradient.setColorAt(1, QColor(255, 0, 0, 0));
        painter.fillRect(kr, gradient);
    }

    // Legal move indicators
    for (const Position& pos : m_legalMoves) {
        QRect rect = squareRect(pos.row, pos.col);
        QPoint center = rect.center();
        Piece* target = m_game->getBoard().getPieceAt(pos);

        if (target) {
            // Capture: draw ring around the square
            painter.setPen(QPen(QColor(0, 0, 0, 50), sz * 0.08));
            painter.setBrush(Qt::NoBrush);
            int inset = static_cast<int>(sz * 0.04);
            painter.drawEllipse(rect.adjusted(inset, inset, -inset, -inset));
            painter.setPen(Qt::NoPen);
        } else {
            // Empty square: draw small dot
            painter.setBrush(legalMoveColor());
            int radius = static_cast<int>(sz * 0.15);
            painter.drawEllipse(center, radius, radius);
        }
    }

    // Draw pieces
    QFont pieceFont;
    pieceFont.setPixelSize(static_cast<int>(sz * 0.78));
    pieceFont.setFamily("Segoe UI Symbol");
    painter.setFont(pieceFont);

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            // Skip the piece being animated
            if (m_animating && m_animPiece.row == r && m_animPiece.col == c)
                continue;

            Piece* piece = m_game->getBoard().getPieceAt(Position{r, c});
            if (!piece) continue;

            QRect rect = squareRect(r, c);
            QString unicode = pieceUnicode(piece->getSymbol());

            // Drop shadow for pieces
            painter.setPen(QColor(0, 0, 0, 60));
            painter.drawText(rect.adjusted(2, 2, 2, 2), Qt::AlignCenter, unicode);

            // Piece itself
            painter.setPen(piece->getColor() == Color::WHITE ? QColor(255, 255, 255) : QColor(30, 30, 30));
            painter.drawText(rect, Qt::AlignCenter, unicode);
        }
    }

    // Draw animating piece
    if (m_animating && m_animPiece.isValid()) {
        Piece* piece = m_game->getBoard().getPieceAt(m_animPiece);
        if (piece) {
            QRect rect = squareRect(m_animPiece.row, m_animPiece.col);
            rect.translate(static_cast<int>(m_animOffset.x()), static_cast<int>(m_animOffset.y()));
            QString unicode = pieceUnicode(piece->getSymbol());

            painter.setPen(QColor(0, 0, 0, 60));
            painter.drawText(rect.adjusted(2, 2, 2, 2), Qt::AlignCenter, unicode);
            painter.setPen(piece->getColor() == Color::WHITE ? QColor(255, 255, 255) : QColor(30, 30, 30));
            painter.drawText(rect, Qt::AlignCenter, unicode);
        }
    }

    // Coordinate labels
    QFont coordFont;
    coordFont.setPixelSize(static_cast<int>(sz * 0.16));
    coordFont.setFamily("Arial");
    coordFont.setBold(true);
    painter.setFont(coordFont);

    for (int i = 0; i < 8; i++) {
        // File labels (a-h) on bottom
        int dispCol = displayCol(i);
        QRect fileRect(offsetX + dispCol * sz, offsetY + boardHeight + 2, sz, static_cast<int>(sz * 0.25));
        bool isLight = (7 + i) % 2 == 0;
        painter.setPen(isLight ? darkSquare() : lightSquare());
        QString fileLabel = QString(QChar('a' + i));
        // Draw at bottom-right of each bottom square
        QRect bottomSquare = squareRect(7, i);
        painter.drawText(bottomSquare.adjusted(0, 0, -3, -2), Qt::AlignRight | Qt::AlignBottom, fileLabel);

        // Rank labels (1-8) on left
        int dispRow = displayRow(i);
        QRect rankRect(offsetX - static_cast<int>(sz * 0.3), offsetY + dispRow * sz, static_cast<int>(sz * 0.25), sz);
        bool isLightRank = (i) % 2 == 0;
        painter.setPen(isLightRank ? darkSquare() : lightSquare());
        QString rankLabel = QString::number(8 - i);
        QRect leftSquare = squareRect(i, 0);
        painter.drawText(leftSquare.adjusted(3, 2, 0, 0), Qt::AlignLeft | Qt::AlignTop, rankLabel);
    }
}

// ─────────────────────────────────────────────
//   MOUSE
// ─────────────────────────────────────────────
void ChessBoardWidget::mousePressEvent(QMouseEvent* event) {
    if (m_animating) return;  // ignore clicks during animation
    if (m_game->getState() == GameState::CHECKMATE ||
        m_game->getState() == GameState::STALEMATE) return;

    Position clicked = boardPosFromPixel(event->pos());
    if (!clicked.isValid()) return;

    emit squareClicked(clicked.row, clicked.col);

    Piece* clickedPiece = m_game->getBoard().getPieceAt(clicked);

    // If a piece is already selected
    if (m_selected.isValid()) {
        // Check if clicked square is a legal move
        bool isLegalTarget = false;
        for (const Position& lm : m_legalMoves) {
            if (lm == clicked) { isLegalTarget = true; break; }
        }

        if (isLegalTarget) {
            // Make the move
            Position from = m_selected;
            Position to = clicked;
            clearSelection();

            // Check if this is a promotion move — show picker
            char promoteTo = 'Q';
            if (m_game->isPromotionMove(from, to)) {
                PromotionDialog dialog(m_game->getCurrentTurn(), this);
                // Position dialog near the promotion square
                QRect sq = squareRect(to.row, to.col);
                QPoint globalPos = mapToGlobal(sq.topLeft());
                dialog.move(globalPos);
                if (dialog.exec() != QDialog::Accepted) {
                    update();
                    return;  // cancelled
                }
                promoteTo = dialog.getChoice();
            }

            if (m_game->makeMove(from, to, promoteTo)) {
                m_lastFrom = from;
                m_lastTo = to;
                updateKingInCheck();
                update();
                emit moveMade();
            }
            return;
        }

        // Clicked on own piece — re-select it
        if (clickedPiece && clickedPiece->getColor() == m_game->getCurrentTurn()) {
            m_selected = clicked;
            m_legalMoves = m_game->getLegalMoves(clicked);
            update();
            return;
        }

        // Clicked elsewhere — deselect
        clearSelection();
        update();
        return;
    }

    // No piece selected — select if it's the current player's piece
    if (clickedPiece && clickedPiece->getColor() == m_game->getCurrentTurn()) {
        m_selected = clicked;
        m_legalMoves = m_game->getLegalMoves(clicked);
        update();
    }
}
