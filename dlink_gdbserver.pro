QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    source/application.cpp \
    source/logout.cpp \
    source/mainwindow.cpp \
    source/memxml.cpp \
    source/misa.cpp \
    source/regxml.cpp \
    source/server.cpp \
    source/target.cpp \
    source/transmit.cpp \
    source/algorithm.cpp

HEADERS += \
    include/algorithm.h \
    include/application.h \
    include/logout.h \
    include/mainwindow.h \
    include/memxml.h \
    include/misa.h \
    include/regxml.h \
    include/server.h \
    include/target.h \
    include/transmit.h

FORMS += \
    forms/mainwindow.ui

RC_ICONS = logo.ico


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
