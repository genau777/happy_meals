QT += network sql widgets
CONFIG += c++11
CONFIG -= console
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = HappyMeals

INCLUDEPATH += include

SOURCES += src/main.cpp \
           src/dishserver.cpp \
           src/storage.cpp \
           src/filters.cpp \
           src/db_singleton.cpp \
           src/functionstoserver.cpp \
           src/usermanager.cpp \
           src/dishmanager.cpp \
           src/clientapi.cpp \
           src/clientsessionmanager.cpp \
           src/mainwindow.cpp \
           src/ingredientsscreen.cpp

HEADERS += include/dishserver.h \
           include/models.h \
           include/storage.h \
           include/filters.h \
           include/db_singleton.h \
           include/functionstoserver.h \
           include/usermanager.h \
           include/dishmanager.h \
           include/clientapi.h \
           include/clientsessionmanager.h \
           include/mainwindow.h \
           include/ingredientsscreen.h

FORMS += mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
