QT += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 debug

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    source/application.cpp \
    source/cpuinfo.cpp \
    source/etrace.cpp \
    source/logout.cpp \
    source/mainwindow.cpp \
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
    include/application.h \
    include/cpuinfo.h \
    include/etrace.h \
    include/logout.h \
    include/mainwindow.h \
    include/memxml.h \
    include/misa.h \
    include/regxml.h \
    include/serial.h \
    include/server.h \
    include/target.h \
    include/transmit.h \
    include/type.h

FORMS += \
    forms/mainwindow.ui

RC_ICONS = logo.ico

build_type =
CONFIG(debug, debug|release) {
    build_type = build_debug
} else {
    build_type = build_release
}

DESTDIR     = $$build_type/out
OBJECTS_DIR = $$build_type/obj
MOC_DIR     = $$build_type/moc
RCC_DIR     = $$build_type/rcc
UI_DIR      = $$build_type/ui

win32:{
    win32-g++ {
        QMAKE_CXXFLAGS += -Wno-deprecated-copy
    }
    win32-msvc*{
    }
    VERSION = $${BUILD_VERSION}.000
    RC_ICONS = "logo.ico"
    QMAKE_TARGET_PRODUCT = "dlink_gdbserver_console"
    QMAKE_TARGET_DESCRIPTION = "dlink_gdbserver_console based on Qt $$[QT_VERSION]"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License v3.0"
}

unix:!macx:{
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie 
    LIBS += -lutil
}

macx:{
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "logo.ico"
    LIBS += -lutil
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
