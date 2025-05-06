#include "MainWindow.h"
#include <QCursor>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>  // For QInputDialog
#include <QLineEdit>     // For QLineEdit
#include <QJsonArray>


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

    webSocket.open(QUrl("ws://192.168.0.103:12345"));


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

        if (type == "insert" || type == "delete") {
            isRemoteChange = true; // Mark as remote change

            if (type == "insert") {
                string id = obj["id"].toString().toStdString();
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

            isRemoteChange = false; }// Reset flag
        else if (type == "file_content" || type == "file_list") {
            handleFileMessage(obj);
        }
    }
}


void MainWindow::onDisconnected() {
    // textEdit->append("[System] Disconnected from server.");
    statusBar()->showMessage("Disconnected", 3000);
}

void MainWindow::onTextChanged() {
    if (isRemoteChange) return;

    QString currentText = textEdit->toPlainText();
    qDebug() << "Current Text:" << currentText;
    qDebug() << "Last Known Text:" << LastKnownText;

    // Handle first character after file load or new file
    if (LastKnownText.isEmpty() && !currentText.isEmpty()) {
        string id = clientId + to_string(charAdded);
        string val = currentText.toStdString();
        std::map<char, int> version_vector_pass = r1.getVersionVector();

        // Insert first character
        r1.insert(id, val, version_vector_pass, "");

        QJsonObject op;
        op["type"] = "insert";
        op["id"] = QString::fromStdString(id);
        op["value"] = currentText;
        op["prev_id"] = "";

        map<char, int> temp_vector = r1.getNodeVersionVector(id);
        QJsonObject versionVec;
        for (const auto& [client, seq] : temp_vector) {
            versionVec[QString(client)] = seq;
        }
        op["version"] = versionVec;

        qDebug() << "First character operation:" << op;
        allOperations.push_back(op);

        charAdded += 1;
        LastKnownText = currentText;

        // Send immediately without debouncing
        debounceTimer.stop();
        sendTextMessage();
        return;
    }

    // Existing text diff logic for non-empty documents
    if (currentText.isEmpty() || LastKnownText.isEmpty()) {
        return;
    }

    int commonPrefix = 0;
    while (commonPrefix < LastKnownText.length() &&
           commonPrefix < currentText.length() &&
           LastKnownText[commonPrefix] == currentText[commonPrefix]) {
        commonPrefix++;
    }

    int commonSuffix = 0;
    while (commonSuffix < LastKnownText.length() - commonPrefix &&
           commonSuffix < currentText.length() - commonPrefix &&
           LastKnownText[LastKnownText.length() - 1 - commonSuffix] ==
               currentText[currentText.length() - 1 - commonSuffix]) {
        commonSuffix++;
    }

    qDebug() << "Diff - Common Prefix:" << commonPrefix << "Common Suffix:" << commonSuffix;

    if (commonPrefix + commonSuffix == LastKnownText.length()) {
        // Insertion case
        QString inserted = currentText.mid(commonPrefix, currentText.length() - commonPrefix - commonSuffix);
        string prev_id = "";
        int pos = commonPrefix;

        if (pos > 0) {
            for (const auto& node : r1.getNodes()) {
                if (!node.is_deleted && r1.getNodeIndex(node) == pos - 1) {
                    prev_id = node.id;
                    break;
                }
            }
        }

        for (int i = 0; i < inserted.length(); i++) {
            string id = clientId + to_string(charAdded + i);
            string val(1, inserted.at(i).toLatin1());
            std::map<char, int> version_vector_pass = r1.getVersionVector();

            r1.insert(id, val, version_vector_pass, prev_id);

            QJsonObject op;
            op["type"] = "insert";
            op["id"] = QString::fromStdString(id);
            op["value"] = QString(val.c_str());
            op["prev_id"] = QString::fromStdString(prev_id);

            map<char, int> temp_vector = r1.getNodeVersionVector(id);
            QJsonObject versionVec;
            for (const auto& [client, seq] : temp_vector) {
                versionVec[QString(client)] = seq;
            }
            op["version"] = versionVec;

            allOperations.push_back(op);
            prev_id = id;
        }

        charAdded += inserted.length();
    }
    else if (commonPrefix + commonSuffix == currentText.length()) {
        // Deletion case
        QString deleted = LastKnownText.mid(commonPrefix, LastKnownText.length() - commonPrefix - commonSuffix);

        for (int i = 0; i < deleted.length(); i++) {
            int pos = commonPrefix;
            for (const auto& node : r1.getNodes()) {
                if (!node.is_deleted && node.value == deleted.mid(i, 1).toStdString() &&
                    r1.getNodeIndex(node) == pos) {
                    QJsonObject op;
                    op["type"] = "delete";
                    op["id"] = QString::fromStdString(node.id);
                    allOperations.push_back(op);
                    r1.remove(node.id);
                    break;
                }
            }
        }
    }

    LastKnownText = currentText;
    r1.print_document();

    // Restart debounce timer
    debounceTimer.stop();
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
        }
    }
    allOperations.clear();
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

    openServerAct = new QAction("Open from &Server", this);
    connect(openServerAct, &QAction::triggered, this, [this]() {
        this->requestFileList(); // Show available files
    });

    saveServerAct = new QAction("&Save to Server", this);
    connect(saveServerAct, &QAction::triggered, this, [this]() {
        if (!this->currentServerFile.isEmpty()) {
            this->saveFileToServer(this->currentServerFile);
        } else {
            bool ok;
            QString filename = QInputDialog::getText(this,
                                                     "Save to Server",
                                                     "Enter filename:",
                                                     QLineEdit::Normal,
                                                     "",
                                                     &ok);

            if (ok && !filename.isEmpty()) {
                this->saveFileToServer(filename);
            }
        }
    });
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

    fileMenu->addAction(openServerAct);
    fileMenu->addAction(saveServerAct);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAct);
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

void MainWindow::loadFileFromServer(const QString& filename) {
    QJsonObject message;
    message["type"] = "request_file";
    message["filename"] = filename;
    webSocket.sendTextMessage(QJsonDocument(message).toJson());
    currentServerFile = filename;
}

void MainWindow::saveFileToServer(const QString& filename) {
    QJsonObject message;
    message["type"] = "save_file";
    message["filename"] = filename;
    message["content"] = textEdit->toPlainText();
    message["crdt_state"] = QString::fromStdString(r1.serializeState());
    message["char_added"] = charAdded;
    webSocket.sendTextMessage(QJsonDocument(message).toJson());
    currentServerFile = filename;
}

void MainWindow::requestFileList() {
    QJsonObject message;
    message["type"] = "list_files";
    webSocket.sendTextMessage(QJsonDocument(message).toJson());
}

void MainWindow::handleFileMessage(const QJsonObject &obj) {
    QString type = obj["type"].toString();

    if (type == "file_content") {
        isRemoteChange = true;
        disconnect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

        QString content = obj["content"].toString();
        QString filename = obj["filename"].toString();

        // Reset state
        if (obj.contains("crdt_state")) {
            r1.deserializeState(obj["crdt_state"].toString().toStdString());
            // Get the maximum sequence number for our client
            charAdded = 0;
            for (const auto& node : r1.getNodes()) {
                if (node.id[0] == clientId) {
                    int currentSeq = stoi(node.id.substr(1));
                    charAdded = max(charAdded, currentSeq);
                }
            }
            charAdded++; // Next available sequence number
        } else {
            r1.initializeFromContent(content.toStdString(), clientId);
            charAdded = content.length() + 1;
        }

        textEdit->setPlainText(content);
        LastKnownText = content;
        currentServerFile = filename;

        // Force an initial sync message
        QTimer::singleShot(100, this, [this]() {
            if (!textEdit->toPlainText().isEmpty()) {
                onTextChanged();
            }
        });

        connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
        isRemoteChange = false;
    } else if (type == "file_list") {
        QJsonArray filesArray = obj["files"].toArray();
        QStringList fileNames;
        for (const QJsonValue &val : filesArray) {
            fileNames.append(val.toString());
        }

        bool ok;
        QString selectedFile = QInputDialog::getItem(this, tr("Open File from Server"),
                                                     tr("Select a file:"), fileNames, 0, false, &ok);
        if (ok && !selectedFile.isEmpty()) {
            QJsonObject requestObj;
            requestObj["type"] = "request_file";
            requestObj["filename"] = selectedFile;
            webSocket.sendTextMessage(QJsonDocument(requestObj).toJson());
        }
    }
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
