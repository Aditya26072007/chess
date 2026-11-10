#pragma once
#include <QDialog>
#include "../chess_oops/Piece.h"

// Chess.com-style promotion picker — shows 4 piece options vertically.
class PromotionDialog : public QDialog {
    Q_OBJECT

public:
    explicit PromotionDialog(Color color, QWidget* parent = nullptr);
    char getChoice() const { return m_choice; }

private:
    char m_choice = 'Q';   // default to Queen
};
