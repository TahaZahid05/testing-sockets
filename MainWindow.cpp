#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextCharFormat>
#include <QFont>
#include <QTextCursor>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QMenuBar>
#include <QStatusBar>
#include <QCursor>
#include <QDebug>

//TO-DO: ADD AUTO-GENERATED ID
    MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentFile(""), clientId('?'), LastKnownText(""), charAdded(0)
{
    // Create central widget and main layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setFixedSize(800, 600);

    // Toolbar Layout
    QToolBar *toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setFixedSize(780,50);

    // Style the toolbar
    toolbar->setStyleSheet(
        "QToolBar {"
        "   background: light gray;"
        "   spacing: 10px;"
        "   padding: 5px;"
        "   border: 2px solid #4a90e2;"
        "   border-radius: 5px;"
        "}"
        "QToolButton {"
        "   padding: 4px;"
        "   icon-size: 24px;"
        "   color: white"
        "   background: transparent"
        "}"
        );

    auto createIconButton = [&](const QString &iconPath, void (MainWindow::*slot)(), bool checkable = false) {
        QToolButton *btn = new QToolButton;
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(24, 24));
        btn->setCheckable(checkable);
        connect(btn, &QToolButton::clicked, this, slot);
        toolbar->addWidget(btn);
        return btn;
    };

    createIconButton(":/icons/icons/rotate-ccw.svg", &MainWindow::onUndo);
    createIconButton(":/icons/icons/rotate-cw.svg", &MainWindow::onRedo);
    createIconButton(":/icons/icons/printer.svg", &MainWindow::onPrint);
    btnBold = createIconButton(":/icons/icons/bold.svg", &MainWindow::onBold, true);
    btnItalic = createIconButton(":/icons/icons/italic.svg", &MainWindow::onItalic, true);
    btnUnderline = createIconButton(":/icons/icons/underline.svg", &MainWindow::onUnderline, true);
    btnAlignLeft = createIconButton(":/icons/icons/align-left.svg", &MainWindow::onAlignLeft, true);
    btnAlignCenter = createIconButton(":/icons/icons/align-center.svg", &MainWindow::onAlignCenter, true);
    btnAlignRight = createIconButton(":/icons/icons/align-right.svg", &MainWindow::onAlignRight, true);

    QButtonGroup *alignGroup = new QButtonGroup(this);
    alignGroup->setExclusive(true);
    alignGroup->addButton(btnAlignLeft);
    alignGroup->addButton(btnAlignCenter);
    alignGroup->addButton(btnAlignRight);

    // Create text edit with styling from file 1
    textEdit = new QTextEdit;
    // textEdit->setStyleSheet(
    //     "QTextEdit {"
    //     "  font-family: 'Consolas', monospace;"
    //     "  font-size: 12pt;"
    //     "  background-color: #f8f8f8;"
    //     "  color: black;"
    //     "  border: 1px solid #ddd;"
    //     "  padding: 10px;"
    //     "}"
    //     );
        
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(textEdit);

    // Bottom buttons (Connect / Save)
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *btnConnect = new QPushButton("🔗 Connect");
    QPushButton *btnSave = new QPushButton("💾 Save");
    connect(btnConnect, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSave);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnConnect);
    buttonLayout->addWidget(btnSave);

    mainLayout->addLayout(buttonLayout);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    LastKnownText = textEdit->toPlainText();

    // WebSocket connections (from your ChatWindow)
    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onMessageReceived);
    connect(&webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(&debounceTimer, &QTimer::timeout, this, &MainWindow::sendTextMessage);
    debounceTimer.setSingleShot(true);

    pongReceived = false;
    currentTimer.setInterval(500);
    connect(&currentTimer, &QTimer::timeout, this, &MainWindow::checkDisconnect);
    connect(&webSocket, &QWebSocket::pong, this, &MainWindow::onPingReceived);
    currentTimer.start();

    // Send message on Enter key (like a chat app)
    // connect(textEdit, &QTextEdit::textChanged, [this]() {
    //     if (textEdit->toPlainText().endsWith("\n")) {
    //         QString msg = textEdit->toPlainText().trimmed();
    //         if (!msg.isEmpty()) {
    //             webSocket.sendTextMessage(msg);
    //             textEdit->append("[You] " + msg);
    //             // textEdit->clear();
    //         }
    //     }
    // });

    textEdit->setFocus();
    textEdit->installEventFilter(this);
    textEdit->setReadOnly(false);

    // Create other UI components from file 1
    createActions();
    createMenus();
    createStatusBar();

    // Window properties
    setWindowTitle("Text Editor");
    resize(800, 600);

// <<<<<<< fixed-delete
//     webSocket.open(QUrl("ws://192.168.0.34:12345"));
// =======
    webSocket.open(QUrl("ws://192.168.0.34:12345"));
// >>>>>>> main
    // webSocket.open(QUrl("wss://46b9-103-125-241-66.ngrok-free.app"));
    // webSocket.open(QUrl("wss://f023-111-88-45-254.ngrok-free.app"));  // Use "wss://" for secure WebSockets
    connect(textEdit, &QTextEdit::cursorPositionChanged, [=]() {
        QTextCharFormat fmt = textEdit->currentCharFormat();
        btnBold->setChecked(fmt.fontWeight() == QFont::Bold);
        btnItalic->setChecked(fmt.fontItalic());
        btnUnderline->setChecked(fmt.fontUnderline());
    });
}

void MainWindow::onPingReceived(quint64 elapsedTime, const QByteArray &payload) {
    if (!isConnected) {
        isConnected = true;
        sendTextMessage();
    }
    // qDebug() << "yay";
    pongReceived = true;
    isConnected = true;
}

void MainWindow::checkDisconnect() {
    if (!pongReceived) {
        // qDebug() << "yes";
        isConnected = false;
        statusBar()->showMessage("Disconnected");
    }
    pongReceived = false;
    webSocket.ping();
}

void MainWindow::onConnected() {
    // textEdit->append("[System] Connected to chat server!");
    webSocket.ping();
    currentTimer.start();
    statusBar()->showMessage("Connected");
    isConnected = true;
}

void MainWindow::onMessageReceived(QString message) {
    qDebug() << "Received message:" << message;

    // Check if message is from server (not JSON)
    if (message.startsWith("[Server] You are Client ID: ")) {
        QString idStr = message.section(':', -1).trimmed();
        if (!idStr.isEmpty() && idStr.length() == 1) {
            clientId = idStr[0].toLatin1();  // Assign client ID
            qDebug() << "Assigned client ID:" << clientId;
        }
        return;
    }

    // Assume remaining messages are JSON-formatted
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON message received";
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    isRemoteChange = true; // Mark as remote change

    if (type == "insert") {
        string id = obj["id"].toString().toStdString();
        if (r1.search(id) == std::nullopt) {
            string value = obj["value"].toString().toStdString();
            string prev_id = obj["prev_id"].toString().toStdString();
            QJsonObject vvJson = obj["version"].toObject();
            map<char, int> node_version_vector;
            for (const auto& key : vvJson.keys()) {
                char client = key[0].toLatin1();
                int seq = vvJson[key].toInt();
                node_version_vector[client] = seq;
            }

            RGA_Node newNode(id, value, node_version_vector, prev_id);
            r1.merge(newNode);
        }
    }
    else if (type == "delete") {
        string id = obj["id"].toString().toStdString();
        r1.remove(id);
    }

    // Preserve cursor position during remote updates
    int oldCursorPos = textEdit->textCursor().position();
    QString newText = QString::fromStdString(r1.print_document());

    disconnect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    textEdit->setPlainText(newText);

    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(std::min<int>(oldCursorPos, static_cast<int>(newText.length())));
    textEdit->setTextCursor(cursor);

    LastKnownText = textEdit->toPlainText();
    connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

    isRemoteChange = false;
}


void MainWindow::onDisconnected() {
    statusBar()->showMessage("Disconnected");
}

void MainWindow::onTextChanged() {
    if (isRemoteChange) {
        return; // Skip processing if change came from remote
    }
    QString currentText = textEdit->toPlainText();
    qDebug() << currentText;
    qDebug() << LastKnownText;
    int commonPrefix = 0;
    while (commonPrefix < LastKnownText.length() &&
           commonPrefix < currentText.length() &&
           LastKnownText[commonPrefix] == currentText[commonPrefix]) {
        qDebug() << LastKnownText[commonPrefix];
        commonPrefix++;
    }
    int commonSuffix = 0;
    while (commonSuffix < LastKnownText.length() - commonPrefix &&
           commonSuffix < currentText.length() - commonPrefix &&
           LastKnownText[LastKnownText.length() - 1 - commonSuffix] == currentText[currentText.length() - 1 - commonSuffix]) {
        commonSuffix++;
    }
    qDebug () << commonPrefix << " " << commonSuffix;
    if(commonPrefix+commonSuffix == LastKnownText.length()){
        QString inserted = currentText.mid(LastKnownText.length()-commonSuffix,1);
        string prev_id = "";
        int pos = LastKnownText.length()-commonSuffix;
        for (const auto& node: r1.getNodes()) {
            if (!node.is_deleted){
                if(r1.getNodeIndex(node) == pos-1) {
                    prev_id = node.id;
                    break;
                }
            }
        }
        string id = clientId + to_string(charAdded);
        string val = inserted.toStdString();
        std::map<char, int> version_vector_pass = r1.getVersionVector();
        r1.insert(id,val,version_vector_pass,prev_id);
        QJsonObject op;
        op["type"] = "insert";
        op["id"] = QString::fromStdString(id);
        op["value"] = inserted;
        op["prev_id"] = QString::fromStdString(prev_id);
        map<char, int> temp_vector = r1.getNodeVersionVector(id);
        QJsonObject versionVec;
        for (const auto& [client, seq] : temp_vector) {
            versionVec[QString(client)] = seq;
        }
        op["version"] = versionVec;
        qDebug() << op["value"];
        allOperations.push_back(op);
    }
    else if(commonPrefix+commonSuffix == currentText.length()) {
        QString deletedStr = LastKnownText.mid(currentText.length()-commonSuffix, 1);
        string deletedValue = deletedStr.toStdString();
        int pos = currentText.length()-commonSuffix;

        for (const auto& node: r1.getNodes()) {
            if(node.value == deletedValue && r1.getNodeIndex(node) == pos) {
                QJsonObject op;
                op["type"] = "delete";
                op["id"] = QString::fromStdString(node.id);
                allOperations.push_back(op);
                r1.remove(node.id);
                break;
            }
        }
    }
    charAdded += 1;
    LastKnownText = currentText;
    r1.print_document();
    debounceTimer.start(3000);
}

void MainWindow::sendTextMessage() {
    for(QJsonObject &obj : allOperations) {
        if (!obj.contains("cursor_pos")) {
            obj["cursor_pos"] = textEdit->textCursor().position();
        }
        if (isConnected) {
            webSocket.sendTextMessage(QJsonDocument(obj).toJson());
            // webSocket.
        }
    }
    if(isConnected) {
        allOperations.clear();
    }
}

// UI-related methods from file 2
void MainWindow::onUndo() { textEdit->undo(); }
void MainWindow::onRedo() { textEdit->redo(); }
void MainWindow::onPrint() {
    QPrinter printer;
    QPrintDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted) textEdit->print(&printer);
}
void MainWindow::onBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(btnBold->isChecked() ? QFont::Bold : QFont::Normal);
    QTextCursor cursor = textEdit->textCursor();
    cursor.mergeCharFormat(fmt);
}
void MainWindow::onItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(btnItalic->isChecked());
    QTextCursor cursor = textEdit->textCursor();
    cursor.mergeCharFormat(fmt);
}
void MainWindow::onUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(btnUnderline->isChecked());
    QTextCursor cursor = textEdit->textCursor();
    cursor.mergeCharFormat(fmt);
}
void MainWindow::onAlignLeft() { textEdit->setAlignment(Qt::AlignLeft); }
void MainWindow::onAlignCenter() { textEdit->setAlignment(Qt::AlignCenter); }
void MainWindow::onAlignRight() { textEdit->setAlignment(Qt::AlignRight); }

void MainWindow::onConnect() {
    // QMessageBox::information(this, "Connect", "Connect clicked");
    webSocket.abort();  // Critical: Releases all socket resources
    QCoreApplication::processEvents();  // Let Qt clean up
    QTimer::singleShot(100, [this]() {
        webSocket.open(QUrl("ws://192.168.0.34:12345"));  // Reconnect
        statusBar()->showMessage("Reconnecting...");
    });
    connect(&webSocket, &QWebSocket::connected, this, [this]() {
        isConnected = true;
        qDebug() << "Reconnected successfully!";
        statusBar()->showMessage("Connected");
        sendTextMessage();  // Safe to send now
    });


}
void MainWindow::onSave() {
    QString file = QFileDialog::getSaveFileName(this, "Save File");
    if (!file.isEmpty()) {
        QFile f(file);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out << textEdit->toPlainText();
            f.close();
        }
    }
}

// File operations from file 1
void MainWindow::createActions()
{
    // File actions
    newAct = new QAction("&New", this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setStatusTip("Create a new file");
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction("&Open...", this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip("Open an existing file");
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    // saveAct = new QAction("&Save", this);
    // saveAct->setShortcut(QKeySequence::Save);
    // saveAct->setStatusTip("Save the document to disk");
    // connect(saveAct, &QAction::triggered, this, &MainWindow::saveAsFile);  // Connect to saveAsFile

    saveAsAct = new QAction("Save &As...", this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip("Save the document under a new name");
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFile);

    exitAct = new QAction("E&xit", this);
    exitAct->setShortcut(QKeySequence::Quit);
    exitAct->setStatusTip("Exit the application");
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Help actions
    aboutAct = new QAction("&About", this);
    aboutAct->setStatusTip("Show the application's About box");
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    // reconnectAct = new QAction("Reconnect", this);
    // reconnectAct->setStatusTip("Reconnect with the server");
    // connect(reconnectAct, &QAction::triggered, this, &MainWindow::reconnect);
}

// void MainWindow::reconnect()
// {
//     // webSocket.open(QUrl("ws://192.168.0.34:12345"));
//     // isConnected = true;
//     // sendTextMessage();


// }

void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);


    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAct);
    // helpMenu->addAction(reconnectAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File");
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

// bool MainWindow::saveFile()
// {
//     if (currentFile.isEmpty()) {
//         return saveAsFile();
//     } else {
//         return saveFile(currentFile);
//     }
// }

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == textEdit && event->type() == QEvent::KeyPress) {
        qDebug() << "Key press event received";
    }
    return QMainWindow::eventFilter(obj, event);
}

bool MainWindow::saveAsFile()
{
    QString fileName;

    // If we have a current file and user clicked "Save" (not "Save As")
    if (sender() == saveAct && !currentFile.isEmpty()) {
        fileName = currentFile;
    }
    else {
        // Otherwise prompt for filename (for "Save As" or first-time save)
        fileName = QFileDialog::getSaveFileName(this, "Save As");
        if (fileName.isEmpty()) {
            return false;
        }
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Text Editor",
                             QString("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName),
                                      file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage("File saved", 2000);
    return true;
}

void MainWindow::about()
{
    QMessageBox::about(this, "About Text Editor",
                       "<h2>Text Editor</h2>"
                       "<p>A simple text editor built with Qt.</p>"
                       "<p>Version 1.0</p>");
}

bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, "Text Editor",
                                                                 "The document has been modified.\n"
                                                                 "Do you want to save your changes?",
                                                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        return saveAsFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Text Editor",
                             QString("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage("File loaded", 2000);
}

// bool MainWindow::saveFile(const QString &fileName)
// {
//     QFile file(fileName);
//     if (!file.open(QFile::WriteOnly | QFile::Text)) {
//         QMessageBox::warning(this, "Text Editor",
//                              QString("Cannot write file %1:\n%2.")
//                                  .arg(QDir::toNativeSeparators(fileName), file.errorString()));
//         return false;
//     }

//     QTextStream out(&file);
//     QApplication::setOverrideCursor(Qt::WaitCursor);
//     out << textEdit->toPlainText();
//     QApplication::restoreOverrideCursor();

//     setCurrentFile(fileName);
//     statusBar()->showMessage("File saved", 2000);
//     return true;
// }

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

MainWindow::~MainWindow()
{
}
