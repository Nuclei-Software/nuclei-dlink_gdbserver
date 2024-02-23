#!/bin/sh

###############################################################################
# 定义Qt目录
QT_DIR=/opt/Qt/6.5.3/gcc_64
###############################################################################
# 定义APP Name
APP_NAME=dlink_gdbserver
###############################################################################
# 定义版本号
MAJARVERSION=$(< ./version.txt cut -d '.' -f 1)
SUBVERSION=$(< ./version.txt cut -d '.' -f 2)
REVISION=$(< ./version.txt cut -d '.' -f 3)
export PATH=$QT_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_DIR/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$QT_DIR/plugins
export QML2_IMPORT_PATH=$QT_DIR/qml
# 合成版本号
APP_VERSION="v"$MAJARVERSION"."$SUBVERSION"."$REVISION
# 编译
rm -rf .qmake.stash Makefile
$QT_DIR/bin/lrelease ./"$APP_NAME".pro
$QT_DIR/bin/qmake -makefile
make
$QT_DIR/bin/lrelease ./"$APP_NAME"_console.pro
$QT_DIR/bin/qmake -makefile
make
cp ./tools/create-dmg/build-dmg.sh ./build_release/out/build-dmg.sh
cp ./tools/create-dmg/installer_background.png ./build_release/out/installer_background.png
cd ./build_release/out
# 打包
$QT_DIR/bin/macdeployqt "$APP_NAME".app
otool -L ./"$APP_NAME".app/Contents/MacOS/"$APP_NAME"
mkdir -p ./"$APP_NAME".app/Contents/MacOS/plugins/"$APP_NAME"
./build-dmg.sh "$APP_NAME"
echo build success!
###############################################################################
