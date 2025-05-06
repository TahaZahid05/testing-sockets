#include "MainWindow.h"
#include <QCursor>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

//TO-DO: ADD AUTO-GENERATED ID
    MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentFile(""), clientId('A'), LastKnownText(""), charAdded(0)
{
    // Create central text edit
    textEdit = new QTextEdit(this);
    textEdit->setStyleSheet(
        "QTextEdit {"
        "  font-family: 'Consolas', monospace;"
        "  font-size: 12pt;"
        "  background-color: #f8f8f8;"
        "  color: black;"
        "  border: 1px solid #ddd;"
        "  padding: 10px;"
        "}"
        );
    setCentralWidget(textEdit);

    LastKnownText = textEdit->toPlainText();

    // WebSocket connections (from your ChatWindow)
    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onMessageReceived);
    connect(&webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(&debounceTimer, &QTimer::timeout, this, &MainWindow::sendTextMessage);
    debounceTimer.setSingleShot(true);

    connect(&webSocket, &QWebSocket::pong, this, &MainWindow::onPingReceived);
    currentTimer.setInterval(8000);
    connect(&currentTimer, &QTimer::timeout, this, &MainWindow::checkDisconnect);

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



    // Create other UI components
    createActions();
    createMenus();
    createStatusBar();

    // Window properties
    setWindowTitle("Text Editor");
    resize(800, 600);

    webSocket.open(QUrl("ws://192.168.0.34:12345"));


}

void MainWindow::onPingReceived(quint64) {
    pongReceived = true;
}

void MainWindow::checkDisconnect() {
    if (!pongReceived) {
        qDebug() << "yes";
        webSocket.abort();
    }
    pongReceived = false;
}

void MainWindow::onConnected() {
    // textEdit->append("[System] Connected to chat server!");
    statusBar()->showMessage("Connected", 3000);
}

void MainWindow::onMessageReceived(QString message) {
    qDebug() << "Received message:" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        isRemoteChange = true; // Mark as remote change

        if (type == "insert") {
            string id = obj["id"].toString().toStdString();
            if (r1.search(id) == std::nullopt){
                string value = obj["value"].toString().toStdString();
                string prev_id = obj["prev_id"].toString().toStdString();
                QJsonObject vvJson = obj["version"].toObject();
                map<char, int> node_version_vector;
                for (const auto& key : vvJson.keys()) {
                    char client = key[0].toLatin1();
                    int seq = vvJson[key].toInt();
                    node_version_vector[client] = seq;
                }

                // RGA remoteOp;
                // remoteOp.insert(id, value, prev_id);
                // qDebug() << remoteOp.print_document();
                // RGA_Node& newNode = remoteOp.getNodes().back();
                // newNode.version_vector = node_version_vector;
                RGA_Node newNode(id, value, node_version_vector, prev_id);

                r1.merge(newNode);
                // qDebug() << "yes";
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

        // Restore cursor to original position
        QTextCursor cursor = textEdit->textCursor();
        cursor.setPosition(std::min<int>(oldCursorPos, static_cast<int>(newText.length())));

        textEdit->setTextCursor(cursor);

        LastKnownText = textEdit->toPlainText();
        connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

        isRemoteChange = false; // Reset flag
    }
}


void MainWindow::onDisconnected() {
    // textEdit->append("[System] Disconnected from server.");
    statusBar()->showMessage("Disconnected", 3000);
}

void MainWindow::onTextChanged() {

    // QString text = textEdit->toPlainText();
    // if (webSocket.isValid() && !text.isEmpty()) {
    //     webSocket.sendTextMessage(text);
    //     qDebug() << "Sent:" << text;
    // }
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
        // qDebug() << currentText.length();
        for (const auto& node: r1.getNodes()) {
            if (!node.is_deleted){
                if(r1.getNodeIndex(node) == pos-1) {
                    prev_id = node.id;
                    break;
                }
            }
        }
        // qDebug() << currentText.length();
        string id = clientId + to_string(charAdded);
        string val = inserted.toStdString();
        std::map<char, int> version_vector_pass = r1.getVersionVector();
        // RGA_Node addNode(id,val,r1.getVersionVecotr(),prev_id);
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
        // qDebug() << currentText.length();
    }
    else if(commonPrefix+commonSuffix == currentText.length()) {
        // Get the deleted string segment (now handles strings instead of chars)
        QString deletedStr = LastKnownText.mid(currentText.length()-commonSuffix, 1);
        string deletedValue = deletedStr.toStdString();
        int pos = currentText.length()-commonSuffix;

        // Find and remove the corresponding node
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
    // qDebug() << "Checking: " << allOperations.size();
    for(QJsonObject &obj : allOperations) {
        if (!obj.contains("cursor_pos")) {
            // Add current cursor position if not already set
            obj["cursor_pos"] = textEdit->textCursor().position();
        }
        if (webSocket.isValid()) {
            webSocket.sendTextMessage(QJsonDocument(obj).toJson());
            // webSocket.
        }
    }
    if(webSocket.isValid()) {
        allOperations.clear();
    }
}


MainWindow::~MainWindow()
{

}

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

    saveAct = new QAction("&Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip("Save the document to disk");
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveAsFile);  // Connect to saveAsFile

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

    reconnectAct = new QAction("Reconnect", this);
    reconnectAct->setStatusTip("Reconnect with the server");
    connect(reconnectAct, &QAction::triggered, this, &MainWindow::reconnect);
}

void MainWindow::reconnect()
{
    webSocket.open(QUrl("ws://192.168.0.34:12345"));
    sendTextMessage();

}


void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    // fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(reconnectAct);
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
