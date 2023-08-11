QT -= gui
QT += network serialport

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main_console.cpp \
    source/application_console.cpp \
    source/cpuinfo.cpp \
    source/etrace.cpp \
    source/memxml.cpp \
    source/misa.cpp \
    source/regxml.cpp \
    source/serial.cpp \
    source/server.cpp \
    source/target.cpp \
    source/transmit.cpp \
    source/algorithm.cpp \
    source/type.cpp

HEADERS += \
    include/algorithm.h \
    include/application_console.h \
    include/cpuinfo.h \
    include/etrace.h \
    include/memxml.h \
    include/misa.h \
    include/regxml.h \
    include/serial.h \
    include/server.h \
    include/target.h \
    include/transmit.h \
    include/type.h

RC_ICONS = logo.ico


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
