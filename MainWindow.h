#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ds2_proj.h"
#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QKeySequence>
#include <QApplication>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QToolButton>
#include <QPushButton>
#include <QToolBar>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextCharFormat>
#include <QFont>
#include <QTextCursor>
#include <QPrinter>
#include <QPrintDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveAsFile();
    void about();
    // void reconnect();

    void onConnected();
    void onMessageReceived(QString message);
    void onDisconnected();
    void onTextChanged();
    void sendTextMessage();

    void onPingReceived(quint64 elapsedTime, const QByteArray &payload);

    void checkDisconnect();
    void onUndo();
    void onRedo();
    void onPrint();
    void onBold();
    void onItalic();
    void onUnderline();
    void onAlignLeft();
    void onAlignCenter();
    void onAlignRight();
    void onSave();
    void onConnect();

private:

    QTimer currentTimer;
    bool pongReceived = false;
    bool isConnected = true;

    bool isRemoteChange = false;
    QTextEdit *textEdit;
    QToolButton *btnBold, *btnItalic, *btnUnderline;
    QToolButton *btnAlignLeft, *btnAlignCenter, *btnAlignRight;

    QString currentFile;
    QWebSocket webSocket;

    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *reconnectAct;
    QTimer debounceTimer;

    void createActions();
    void createMenus();
    void createStatusBar();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    RGA r1;
    int charAdded;
    char clientId;
    QString LastKnownText;
    vector<QJsonObject> allOperations;
};

#endif 
