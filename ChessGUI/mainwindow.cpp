#include "mainwindow.h"
#include "chessboardwidget.h"
#include "movepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Chess — Two Player");
    setMinimumSize(960, 700);
    resize(1100, 750);

    // Dark theme for the whole window
    setStyleSheet(
        "QMainWindow { background-color: #161512; }"
    );

    setupUI();
    updateStatusDisplay();
}

MainWindow::~MainWindow() = default;

QString MainWindow::pieceUnicode(char symbol) const {
    switch (symbol) {
        case 'K': return QString::fromUtf8("\u2654");
        case 'Q': return QString::fromUtf8("\u2655");
        case 'R': return QString::fromUtf8("\u2656");
        case 'B': return QString::fromUtf8("\u2657");
        case 'N': return QString::fromUtf8("\u2658");
        case 'P': return QString::fromUtf8("\u2659");
        case 'k': return QString::fromUtf8("\u265A");
        case 'q': return QString::fromUtf8("\u265B");
        case 'r': return QString::fromUtf8("\u265C");
        case 'b': return QString::fromUtf8("\u265D");
        case 'n': return QString::fromUtf8("\u265E");
        case 'p': return QString::fromUtf8("\u265F");
        default:  return QString();
    }
}

QWidget* MainWindow::createPlayerPanel(const QString& name, QLabel*& capturedLabel,
                                         QLabel*& playerLabel, bool isTop) {
    QWidget* panel = new QWidget;
    panel->setFixedHeight(42);
    panel->setStyleSheet("background-color: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(panel);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(10);

    // Player avatar circle
    QLabel* avatar = new QLabel;
    avatar->setFixedSize(28, 28);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        QString("background-color: %1;"
                "border-radius: 14px;"
                "color: %2;"
                "font-weight: bold;"
                "font-size: 14px;"
                "font-family: 'Segoe UI';")
            .arg(isTop ? "#555" : "#ddd")
            .arg(isTop ? "#fff" : "#333")
    );
    avatar->setText(name.left(1));
    layout->addWidget(avatar);

    // Player name
    playerLabel = new QLabel(name);
    playerLabel->setStyleSheet(
        "color: #e0e0e0;"
        "font-family: 'Segoe UI';"
        "font-size: 14px;"
        "font-weight: bold;"
    );
    layout->addWidget(playerLabel);

    // Captured pieces display
    capturedLabel = new QLabel;
    capturedLabel->setStyleSheet(
        "color: #b0b0b0;"
        "font-size: 16px;"
        "font-family: 'Segoe UI Symbol';"
    );
    layout->addWidget(capturedLabel);

    layout->addStretch();
    return panel;
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget;
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(16);

    // ─── Left side: Board + player panels ───
    QVBoxLayout* boardSection = new QVBoxLayout;
    boardSection->setSpacing(0);

    // Top player panel (Black)
    QWidget* blackPanel = createPlayerPanel("Black", m_blackCapturedLabel,
                                             m_blackPlayerLabel, true);
    boardSection->addWidget(blackPanel);

    // Chess board
    m_boardWidget = new ChessBoardWidget(&m_game);
    connect(m_boardWidget, &ChessBoardWidget::moveMade, this, &MainWindow::onMoveMade);

    // Board container with border
    QFrame* boardFrame = new QFrame;
    boardFrame->setStyleSheet(
        "QFrame {"
        "    border: 3px solid #302e2b;"
        "    border-radius: 4px;"
        "    background-color: #302e2b;"
        "}"
    );
    QVBoxLayout* frameLayout = new QVBoxLayout(boardFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(m_boardWidget);
    boardSection->addWidget(boardFrame, 1);

    // Bottom player panel (White)
    QWidget* whitePanel = createPlayerPanel("White", m_whiteCapturedLabel,
                                             m_whitePlayerLabel, false);
    boardSection->addWidget(whitePanel);

    mainLayout->addLayout(boardSection, 1);

    // ─── Right side: Move panel + controls ───
    QVBoxLayout* rightSection = new QVBoxLayout;
    rightSection->setSpacing(0);

    // Move panel
    m_movePanel = new MovePanel(&m_game);
    QFrame* movePanelFrame = new QFrame;
    movePanelFrame->setStyleSheet(
        "QFrame {"
        "    border: 1px solid #3a3835;"
        "    border-radius: 6px;"
        "    background-color: #262522;"
        "}"
    );
    QVBoxLayout* mpLayout = new QVBoxLayout(movePanelFrame);
    mpLayout->setContentsMargins(0, 0, 0, 0);
    mpLayout->addWidget(m_movePanel);
    rightSection->addWidget(movePanelFrame, 1);

    // Status label
    m_statusLabel = new QLabel;
    m_statusLabel->setFixedHeight(36);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #f0f0f0;"
        "font-family: 'Segoe UI';"
        "font-size: 13px;"
        "font-weight: bold;"
        "background-color: #302e2b;"
        "border-radius: 4px;"
        "margin-top: 8px;"
    );
    rightSection->addWidget(m_statusLabel);

    // Turn indicator
    m_turnLabel = new QLabel;
    m_turnLabel->setFixedHeight(30);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet(
        "color: #bababa;"
        "font-family: 'Segoe UI';"
        "font-size: 12px;"
        "margin-top: 4px;"
    );
    rightSection->addWidget(m_turnLabel);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    QString buttonStyle =
        "QPushButton {"
        "    background-color: #3c3a36;"
        "    color: #e0e0e0;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-family: 'Segoe UI';"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4a4845;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #555350;"
        "}";

    QString undoStyle =
        "QPushButton {"
        "    background-color: #81b64c;"
        "    color: #fff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-family: 'Segoe UI';"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #95c95f;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #6ea33d;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #3c3a36;"
        "    color: #666;"
        "}";

    m_undoButton = new QPushButton("↩ Undo");
    m_undoButton->setStyleSheet(undoStyle);
    m_undoButton->setFixedHeight(38);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setEnabled(false);
    connect(m_undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);

    m_flipButton = new QPushButton("⟳ Flip");
    m_flipButton->setStyleSheet(buttonStyle);
    m_flipButton->setFixedHeight(38);
    m_flipButton->setCursor(Qt::PointingHandCursor);
    connect(m_flipButton, &QPushButton::clicked, this, &MainWindow::onFlipBoardClicked);

    m_newGameButton = new QPushButton("New Game");
    m_newGameButton->setStyleSheet(buttonStyle);
    m_newGameButton->setFixedHeight(38);
    m_newGameButton->setCursor(Qt::PointingHandCursor);
    connect(m_newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);

    buttonLayout->addWidget(m_undoButton);
    buttonLayout->addWidget(m_flipButton);
    buttonLayout->addWidget(m_newGameButton);

    rightSection->addLayout(buttonLayout);

    mainLayout->addLayout(rightSection, 0);
}

void MainWindow::onMoveMade() {
    updateStatusDisplay();
    updateCapturedPieces();
    m_movePanel->refresh();
    m_undoButton->setEnabled(m_game.getMoveCount() > 0);

    // Show game-over dialogs
    if (m_game.getState() == GameState::CHECKMATE) {
        QString winner = (m_game.getCurrentTurn() == Color::WHITE) ? "Black" : "White";
        QTimer::singleShot(300, this, [this, winner]() {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Checkmate!");
            msgBox.setText(QString("<b style='font-size:16px;'>%1 wins by Checkmate!</b>").arg(winner));
            msgBox.setInformativeText("Would you like to start a new game?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setStyleSheet(
                "QMessageBox { background-color: #262522; }"
                "QLabel { color: #e0e0e0; font-family: 'Segoe UI'; }"
                "QPushButton { background-color: #81b64c; color: white; border-radius: 4px; "
                "padding: 6px 20px; font-weight: bold; }"
                "QPushButton:hover { background-color: #95c95f; }"
            );
            if (msgBox.exec() == QMessageBox::Yes) {
                onNewGameClicked();
            }
        });
    } else if (m_game.getState() == GameState::STALEMATE) {
        QTimer::singleShot(300, this, [this]() {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Stalemate!");
            msgBox.setText("<b style='font-size:16px;'>Game drawn by Stalemate</b>");
            msgBox.setInformativeText("Would you like to start a new game?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setStyleSheet(
                "QMessageBox { background-color: #262522; }"
                "QLabel { color: #e0e0e0; font-family: 'Segoe UI'; }"
                "QPushButton { background-color: #81b64c; color: white; border-radius: 4px; "
                "padding: 6px 20px; font-weight: bold; }"
                "QPushButton:hover { background-color: #95c95f; }"
            );
            if (msgBox.exec() == QMessageBox::Yes) {
                onNewGameClicked();
            }
        });
    }
}

void MainWindow::onUndoClicked() {
    if (m_game.undoLastMove()) {
        m_boardWidget->refreshBoard();
        m_movePanel->refresh();
        updateStatusDisplay();
        updateCapturedPieces();
        m_undoButton->setEnabled(m_game.getMoveCount() > 0);
    }
}

void MainWindow::onNewGameClicked() {
    m_game.reset();
    m_boardWidget->refreshBoard();
    m_movePanel->clear();
    updateStatusDisplay();
    updateCapturedPieces();
    m_undoButton->setEnabled(false);
}

void MainWindow::onFlipBoardClicked() {
    m_boardWidget->setFlipped(!m_boardWidget->isFlipped());
}

void MainWindow::updateStatusDisplay() {
    GameState state = m_game.getState();
    Color turn = m_game.getCurrentTurn();
    QString turnStr = (turn == Color::WHITE) ? "White" : "Black";

    switch (state) {
        case GameState::ONGOING:
            m_statusLabel->setText(turnStr + "'s turn to move");
            m_statusLabel->setStyleSheet(
                "color: #f0f0f0; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
                "background-color: #302e2b; border-radius: 4px; margin-top: 8px;"
            );
            break;
        case GameState::CHECK:
            m_statusLabel->setText("⚠ " + turnStr + " is in CHECK!");
            m_statusLabel->setStyleSheet(
                "color: #ff6b6b; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
                "background-color: #3d2020; border-radius: 4px; margin-top: 8px;"
                "border: 1px solid #662222;"
            );
            break;
        case GameState::CHECKMATE: {
            QString winner = (turn == Color::WHITE) ? "Black" : "White";
            m_statusLabel->setText("♛ CHECKMATE — " + winner + " wins!");
            m_statusLabel->setStyleSheet(
                "color: #81b64c; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
                "background-color: #1e2a15; border-radius: 4px; margin-top: 8px;"
                "border: 1px solid #2a4020;"
            );
            break;
        }
        case GameState::STALEMATE:
            m_statusLabel->setText("½ STALEMATE — Draw!");
            m_statusLabel->setStyleSheet(
                "color: #e8b931; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
                "background-color: #2a2615; border-radius: 4px; margin-top: 8px;"
                "border: 1px solid #403820;"
            );
            break;
    }

    // Turn indicator shows move count
    int moveNum = (m_game.getMoveCount() / 2) + 1;
    m_turnLabel->setText(QString("Move %1").arg(moveNum));

    // Highlight active player name
    if (state != GameState::CHECKMATE && state != GameState::STALEMATE) {
        if (turn == Color::WHITE) {
            m_whitePlayerLabel->setStyleSheet(
                "color: #81b64c; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
            );
            m_blackPlayerLabel->setStyleSheet(
                "color: #888; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
            );
        } else {
            m_blackPlayerLabel->setStyleSheet(
                "color: #81b64c; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
            );
            m_whitePlayerLabel->setStyleSheet(
                "color: #888; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
            );
        }
    }
}

void MainWindow::updateCapturedPieces() {
    const auto& captured = m_game.getCapturedPieces();

    QString whiteCaptured;  // black pieces captured by white
    QString blackCaptured;  // white pieces captured by black

    for (const auto& [sym, color] : captured) {
        QString uni = pieceUnicode(sym);
        if (color == Color::BLACK) {
            whiteCaptured += uni + " ";
        } else {
            blackCaptured += uni + " ";
        }
    }

    m_whiteCapturedLabel->setText(whiteCaptured.trimmed());
    m_blackCapturedLabel->setText(blackCaptured.trimmed());
}
