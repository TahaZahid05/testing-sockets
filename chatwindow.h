#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatWindow;
}
QT_END_NAMESPACE

class ChatWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void onConnected();
    void onMessageReceived(QString message);
    void onDisconnected();
    void onTextChanged();  // New slot for real-time updates

private:
    Ui::ChatWindow *ui;
    QWebSocket webSocket;
};

#endif // CHATWINDOW_H
