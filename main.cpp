#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application style
    app.setStyle("Fusion");

    // Create and show main window
    MainWindow mainWin;
    mainWin.show();

    return app.exec();
}
