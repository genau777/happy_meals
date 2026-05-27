QT += core network sql

CONFIG += c++17 console
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = HappyMealsServer

INCLUDEPATH += include

SOURCES += \
    src/server_main.cpp \
    src/db_singleton.cpp \
    src/dishserver.cpp \
    src/dishmanager.cpp \
    src/storage.cpp \
    src/filters.cpp \
    src/functionstoserver.cpp \
    src/usermanager.cpp

HEADERS += \
    include/db_singleton.h \
    include/dishserver.h \
    include/dishmanager.h \
    include/models.h \
    include/storage.h \
    include/filters.h \
    include/functionstoserver.h \
    include/usermanager.h

DISTFILES += \
    data/HappyMealsDB.sqlite \
    data/dishes.sqlite
