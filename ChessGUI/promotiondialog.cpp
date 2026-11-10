#include "promotiondialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>

PromotionDialog::PromotionDialog(Color color, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Promote Pawn");
    setFixedSize(100, 340);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    setStyleSheet(
        "QDialog {"
        "    background-color: #302e2b;"
        "    border: 2px solid #81b64c;"
        "    border-radius: 8px;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 8, 6, 8);
    layout->setSpacing(4);

    // Piece options: Queen, Rook, Bishop, Knight
    struct PieceOption {
        char symbol;
        QString unicode;
        QString name;
    };

    std::vector<PieceOption> options;
    if (color == Color::WHITE) {
        options = {
            {'Q', QString::fromUtf8("\u2655"), "Queen"},
            {'R', QString::fromUtf8("\u2656"), "Rook"},
            {'B', QString::fromUtf8("\u2657"), "Bishop"},
            {'N', QString::fromUtf8("\u2658"), "Knight"}
        };
    } else {
        options = {
            {'Q', QString::fromUtf8("\u265B"), "Queen"},
            {'R', QString::fromUtf8("\u265C"), "Rook"},
            {'B', QString::fromUtf8("\u265D"), "Bishop"},
            {'N', QString::fromUtf8("\u265E"), "Knight"}
        };
    }

    for (const auto& opt : options) {
        QPushButton* btn = new QPushButton(opt.unicode);
        btn->setFixedSize(80, 72);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(opt.name);

        QFont font("Segoe UI Symbol", 36);
        btn->setFont(font);

        QString textColor = (color == Color::WHITE) ? "#ffffff" : "#1e1e1e";
        btn->setStyleSheet(
            QString(
                "QPushButton {"
                "    background-color: #3c3a36;"
                "    color: %1;"
                "    border: none;"
                "    border-radius: 6px;"
                "}"
                "QPushButton:hover {"
                "    background-color: #81b64c;"
                "}"
                "QPushButton:pressed {"
                "    background-color: #6ea33d;"
                "}"
            ).arg(textColor)
        );

        char choice = opt.symbol;
        connect(btn, &QPushButton::clicked, this, [this, choice]() {
            m_choice = choice;
            accept();
        });

        layout->addWidget(btn, 0, Qt::AlignHCenter);
    }
}
