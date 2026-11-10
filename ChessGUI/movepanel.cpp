#include "movepanel.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QFont>
#include <QScrollBar>
#include <QTimer>

MovePanel::MovePanel(Game* game, QWidget* parent)
    : QWidget(parent), m_game(game)
{
    setMinimumWidth(260);
    setMaximumWidth(320);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Title bar
    QLabel* title = new QLabel("  Moves");
    title->setFixedHeight(44);
    QFont titleFont("Segoe UI", 13, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet(
        "background-color: #302e2b;"
        "color: #bababa;"
        "padding-left: 12px;"
        "border-bottom: 1px solid #3a3835;"
    );
    mainLayout->addWidget(title);

    // Scroll area for moves
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: #262522; }"
        "QScrollBar:vertical {"
        "    background: #262522;"
        "    width: 8px;"
        "    margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #4a4845;"
        "    min-height: 20px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0;"
        "}"
    );

    m_moveContainer = new QWidget;
    m_moveContainer->setStyleSheet("background-color: #262522;");
    m_moveLayout = new QVBoxLayout(m_moveContainer);
    m_moveLayout->setContentsMargins(0, 0, 0, 0);
    m_moveLayout->setSpacing(0);
    m_moveLayout->addStretch();

    m_scrollArea->setWidget(m_moveContainer);
    mainLayout->addWidget(m_scrollArea);
}

void MovePanel::clear() {
    // Remove all widgets from the move layout except the stretch
    QLayoutItem* item;
    while ((item = m_moveLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_moveLayout->addStretch();
}

void MovePanel::refresh() {
    rebuildMoveList();
}

void MovePanel::rebuildMoveList() {
    clear();

    const auto& history = m_game->getHistory();
    if (history.empty()) return;

    // Group moves into pairs (white, black)
    for (size_t i = 0; i < history.size(); i += 2) {
        QWidget* row = new QWidget;
        row->setFixedHeight(32);

        int moveNum = static_cast<int>(i / 2) + 1;
        bool isLastPair = (i + 2 >= history.size());

        // Alternate row background
        QString rowBg = (moveNum % 2 == 0) ? "#2b2926" : "#262522";
        if (isLastPair) {
            rowBg = "#32302d";  // highlight the latest move row
        }
        row->setStyleSheet(QString("background-color: %1;").arg(rowBg));

        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(0);

        // Move number
        QLabel* numLabel = new QLabel(QString::number(moveNum) + ".");
        numLabel->setFixedWidth(40);
        numLabel->setAlignment(Qt::AlignCenter);
        numLabel->setStyleSheet(
            "color: #7b7976;"
            "font-family: 'Segoe UI';"
            "font-size: 13px;"
        );
        rowLayout->addWidget(numLabel);

        // White's move
        QLabel* whiteMove = new QLabel(QString::fromStdString(history[i].notation));
        whiteMove->setFixedWidth(100);
        whiteMove->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        whiteMove->setStyleSheet(
            "color: #e0e0e0;"
            "font-family: 'Segoe UI';"
            "font-size: 14px;"
            "font-weight: bold;"
            "padding-left: 10px;"
        );
        rowLayout->addWidget(whiteMove);

        // Black's move (if it exists)
        if (i + 1 < history.size()) {
            QLabel* blackMove = new QLabel(QString::fromStdString(history[i + 1].notation));
            blackMove->setFixedWidth(100);
            blackMove->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            blackMove->setStyleSheet(
                "color: #e0e0e0;"
                "font-family: 'Segoe UI';"
                "font-size: 14px;"
                "font-weight: bold;"
                "padding-left: 10px;"
            );
            rowLayout->addWidget(blackMove);
        } else {
            rowLayout->addStretch();
        }

        rowLayout->addStretch();
        // Insert before the final stretch
        m_moveLayout->insertWidget(m_moveLayout->count() - 1, row);
    }

    // Scroll to bottom
    QTimer::singleShot(10, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum()
        );
    });
}

void MovePanel::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0x26, 0x25, 0x22));
    QWidget::paintEvent(event);
}
