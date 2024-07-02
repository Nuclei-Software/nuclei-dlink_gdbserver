#!/bin/env bash

set -e

###############################################################################
# 定义Qt目录
QT_DIR=${QT_DIR:-/home/share/devtools/qt/6.6.2/gcc_64}

###############################################################################
# 定义APP Name
APP_NAME=dlink_gdbserver

###############################################################################
# 定义版本号
MAJARVERSION=$(< ./version.txt cut -d '.' -f 1)
SUBVERSION=$(<  ./version.txt cut -d '.' -f 2)
REVISION=$(< ./version.txt cut -d '.' -f 3)

# 设置编译环境
export PATH=$QT_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_DIR/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$QT_DIR/plugins
export QML2_IMPORT_PATH=$QT_DIR/qml

# 合成版本号
APP_VERSION="v"$MAJARVERSION"."$SUBVERSION"."$REVISION

# 编译
rm -rf .qmake.stash Makefile

echo "Build DLink GDB Server GUI Version"
lrelease ./"$APP_NAME".pro
qmake ./"$APP_NAME".pro -spec linux-g++ CONFIG+=qtquickcompiler
make clean
make -j8

echo "Build DLink GDB Server Console Version"
lrelease ./"$APP_NAME"_console.pro
qmake ./"$APP_NAME"_console.pro -spec linux-g++ CONFIG+=qtquickcompiler
make clean
make -j8

echo "Test ${APP_NAME}_console"

./build_debug/out/dlink_gdbserver_console -v
./build_debug/out/dlink_gdbserver_console -h
./build_debug/out/dlink_gdbserver_console || true

echo "If no big changes in source code structure, just rebuild this app by commands below:"
echo "make -j"
echo "Test it via command below:"
echo "./build_debug/out/dlink_gdbserver_console"
