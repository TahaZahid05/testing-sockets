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
    explicit MainWindow(char clientId = 'A', QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // File operations
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void about();

    // WebSocket operations
    void onConnected();
    void onMessageReceived(QString message);
    void onDisconnected();
    void onTextChanged();
    void sendTextMessage();

    // Formatting operations
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
    // UI Components
    bool isRemoteChange = false;
    QTextEdit *textEdit;
    QToolButton *btnBold, *btnItalic, *btnUnderline;
    QToolButton *btnAlignLeft, *btnAlignCenter, *btnAlignRight;

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
