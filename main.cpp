#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Use first command-line argument as clientId (default: 'A')
    char clientId = (argc > 1) ? argv[1][0] : 'A';

    MainWindow window(clientId);
    window.show();
    return app.exec();
}
