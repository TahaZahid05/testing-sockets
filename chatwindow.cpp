#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QDebug>

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ChatWindow) {
    ui->setupUi(this);

    // Connect WebSocket signals
    connect(&webSocket, &QWebSocket::connected, this, &ChatWindow::onConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &ChatWindow::onMessageReceived);
    connect(&webSocket, &QWebSocket::disconnected, this, &ChatWindow::onDisconnected);

    // Connect textChanged signal for real-time sending
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatWindow::onTextChanged);

    qDebug() << "Connecting to WebSocket server...";
    webSocket.open(QUrl("ws://192.168.0.34:12345"));  // Connect to Python WebSocket server
}

ChatWindow::~ChatWindow() {
    delete ui;
}

void ChatWindow::onConnected() {
    qDebug() << "Connected to WebSocket server!";
}

void ChatWindow::onMessageReceived(QString message) {
    qDebug() << "Message from server:" << message;
    ui->chatDisplay->append(" " + message);
}

void ChatWindow::onDisconnected() {
    qDebug() << "Disconnected from server.";
}

// New function to send messages in real-time
void ChatWindow::onTextChanged() {
    QString text = ui->messageInput->text();
    if (webSocket.isValid() && !text.isEmpty()) {
        webSocket.sendTextMessage(text);
        qDebug() << "Sent:" << text;
    }
}
