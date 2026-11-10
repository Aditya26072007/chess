#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <vector>
#include <string>
#include "../chess_oops/Game.h"

// Side panel showing move history in chess.com-style two-column layout.
class MovePanel : public QWidget {
    Q_OBJECT

public:
    explicit MovePanel(Game* game, QWidget* parent = nullptr);
    void refresh();     // rebuild move list from game history
    void clear();       // clear all moves

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Game* m_game;
    QScrollArea* m_scrollArea;
    QWidget* m_moveContainer;
    QVBoxLayout* m_moveLayout;

    void rebuildMoveList();
};
