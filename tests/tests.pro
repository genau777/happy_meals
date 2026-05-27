QT += testlib core

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = HappyMealsTests

INCLUDEPATH += ../include

SOURCES += \
    test_ingredientfilter.cpp \
    ../src/filters.cpp

HEADERS += \
    ../include/models.h \
    ../include/filters.h