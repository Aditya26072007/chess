#pragma once
#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPoint>
#include <vector>
#include "../chess_oops/Game.h"

// The interactive chessboard — handles clicks, highlights, animations, and rendering.
class ChessBoardWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QPointF animOffset READ animOffset WRITE setAnimOffset)

public:
    explicit ChessBoardWidget(Game* game, QWidget* parent = nullptr);

    void refreshBoard();            // repaint after external state change
    void setFlipped(bool flipped);  // toggle board orientation
    bool isFlipped() const { return m_flipped; }

signals:
    void moveMade();                // emitted after a successful move
    void moveUndone();              // emitted after undo
    void squareClicked(int row, int col);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    QSize sizeHint() const override { return QSize(640, 640); }
    QSize minimumSizeHint() const override { return QSize(320, 320); }

private:
    Game* m_game;
    bool m_flipped = false;

    // Selection state
    Position m_selected{-1, -1};            // currently selected square
    std::vector<Position> m_legalMoves;     // legal moves for the selected piece

    // Last-move highlight
    Position m_lastFrom{-1, -1};
    Position m_lastTo{-1, -1};

    // King in check highlight
    Position m_kingInCheck{-1, -1};

    // Animation
    QPointF m_animOffset{0, 0};
    bool m_animating = false;
    Position m_animPiece{-1, -1};       // which piece is animating
    QPropertyAnimation* m_animation = nullptr;

    QPointF animOffset() const { return m_animOffset; }
    void setAnimOffset(QPointF p) { m_animOffset = p; update(); }

    // Helpers
    int squareSize() const;
    QRect squareRect(int row, int col) const;
    Position boardPosFromPixel(const QPoint& pixel) const;
    int displayRow(int row) const { return m_flipped ? (7 - row) : row; }
    int displayCol(int col) const { return m_flipped ? (7 - col) : col; }
    void clearSelection();
    void updateKingInCheck();
    QString pieceUnicode(char symbol) const;

    // Colors — chess.com green theme
    QColor lightSquare() const { return QColor(234, 235, 200); }
    QColor darkSquare()  const { return QColor(119, 149, 86);  }
    QColor selectedColor()   const { return QColor(247, 247, 105, 180); }
    QColor legalMoveColor()  const { return QColor(0, 0, 0, 40); }
    QColor lastMoveColor()   const { return QColor(255, 255, 0, 80); }
    QColor checkColor()      const { return QColor(235, 50, 50, 180); }
};
