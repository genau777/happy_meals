QT += network
CONFIG += c++11 console
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = HappyMeals

SOURCES += main.cpp dishserver.cpp storage.cpp filters.cpp
HEADERS += dishserver.h models.h storage.h filters.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
