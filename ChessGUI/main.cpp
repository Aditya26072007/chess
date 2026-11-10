#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application-wide defaults
    app.setApplicationName("Chess");
    app.setOrganizationName("ChessProject");

    // Default font
    QFont defaultFont("Segoe UI", 10);
    app.setFont(defaultFont);

    // Global dark palette
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}