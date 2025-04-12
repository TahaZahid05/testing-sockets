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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(char clientId = 'A', QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void about();

    void onConnected();
    void onMessageReceived(QString message);
    void onDisconnected();
    // void sendMessage();
    void onTextChanged();
    void sendTextMessage();

private:
    // UI Components
    bool isRemoteChange = false;
    QTextEdit *textEdit;

    // File handling
    QString currentFile;
    QWebSocket webSocket;

    // Actions
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QTimer debounceTimer;

    void createActions();
    void createMenus();
    void createStatusBar();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    // RGA related
    RGA r1;
    int charAdded;
    char clientId;
    QString LastKnownText;
    vector<QJsonObject> allOperations;
};

#endif // MAINWINDOW_H
