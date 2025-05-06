#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    MainWindow mainWin;
    mainWin.show();

    return app.exec();
}
