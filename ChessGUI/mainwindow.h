#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "../chess_oops/Game.h"

class ChessBoardWidget;
class MovePanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onMoveMade();
    void onUndoClicked();
    void onNewGameClicked();
    void onFlipBoardClicked();

private:
    Game m_game;
    ChessBoardWidget* m_boardWidget;
    MovePanel* m_movePanel;

    // Status / info labels
    QLabel* m_turnLabel;
    QLabel* m_statusLabel;
    QLabel* m_whiteCapturedLabel;
    QLabel* m_blackCapturedLabel;
    QLabel* m_whitePlayerLabel;
    QLabel* m_blackPlayerLabel;

    // Buttons
    QPushButton* m_undoButton;
    QPushButton* m_newGameButton;
    QPushButton* m_flipButton;

    void updateStatusDisplay();
    void updateCapturedPieces();
    QString pieceUnicode(char symbol) const;
    void setupUI();
    QWidget* createPlayerPanel(const QString& name, QLabel*& capturedLabel,
                                QLabel*& playerLabel, bool isTop);
};

#endif // MAINWINDOW_H
