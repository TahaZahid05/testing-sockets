QT       += core gui websockets
QT       += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chatapp
TEMPLATE = app

SOURCES += main.cpp \
           chatwindow.cpp

HEADERS  += chatwindow.h

FORMS    += chatwindow.ui

# Ensure proper C++ standard
CONFIG += c++17

# Include Qt WebSockets module
QT += websockets
