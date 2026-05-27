QT += core gui network widgets

CONFIG += c++17
CONFIG -= console
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = HappyMealsClient

INCLUDEPATH += include

SOURCES += \
    src/main.cpp \
    src/clientapi.cpp \
    src/clientsessionmanager.cpp \
    src/mainwindow.cpp \
    src/ingredientsscreen.cpp

HEADERS += \
    include/clientapi.h \
    include/clientsessionmanager.h \
    include/mainwindow.h \
    include/ingredientsscreen.h

FORMS += \
    mainwindow.ui

macx {
    CONFIG += app_bundle
}
