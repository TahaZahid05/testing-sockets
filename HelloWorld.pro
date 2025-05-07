QT += widgets
QT += printsupport
TARGET = TextEditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h \
    ds2_proj.h

FORMS += \
    MainWindow.ui

CONFIG += c++17

# Include Qt WebSockets module
QT += websockets

RESOURCES += \
    resources.qrc
