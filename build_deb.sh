#!/bin/env bash

set -e

###############################################################################
# 定义Qt目录
QT_DIR=/opt/Qt/6.5.3/gcc_64
###############################################################################
# 定义APP Name
APP_NAME=dlink_gdbserver
###############################################################################
# 定义版本号
MAJARVERSION=$(< ./version.txt cut -d '.' -f 1)
SUBVERSION=$(<  ./version.txt cut -d '.' -f 2)
REVISION=$(< ./version.txt cut -d '.' -f 3)
export PATH=$QT_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_DIR/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$QT_DIR/plugins
export QML2_IMPORT_PATH=$QT_DIR/qml
# 合成版本号
APP_VERSION="v"$MAJARVERSION"."$SUBVERSION"."$REVISION
# 编译
rm -rf .qmake.stash Makefile
lrelease ./"$APP_NAME".pro
qmake ./"$APP_NAME".pro -spec linux-g++ CONFIG+=qtquickcompiler
make clean
make -j8
lrelease ./"$APP_NAME"_console.pro
qmake ./"$APP_NAME"_console.pro -spec linux-g++ CONFIG+=qtquickcompiler
make clean
make -j8
# clean打包目录
rm -rf ./dpkg/Linux_"$APP_VERSION"_x86_64
rm -f ./dpkg/Linux_"$APP_VERSION"_x86_64.deb
# 构建打包目录
mkdir ./dpkg/Linux_"$APP_VERSION"_x86_64
# 使用linuxdeployqt拷贝依赖so库到打包目录
export QMAKE=$QT_DIR/bin/qmake
pwd
ls -l ./build_debug/out/
./tools/linuxdeploy-x86_64.AppImage --executable=./build_debug/out/"$APP_NAME" --appdir=./dpkg/Linux_"$APP_VERSION"_x86_64/opt --plugin=qt
rm -rf ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/apprun-hooks
mv ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/usr ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"
mv ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/bin/"$APP_NAME" ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/"$APP_NAME"
mv ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/bin/qt.conf ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/qt.conf

cp ./build_debug/out/"$APP_NAME"_console ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/"$APP_NAME"_console

rm -rf ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/bin
sed -i "s/Prefix = ..\//Prefix = .\//g" ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/qt.conf
chrpath -r "\$ORIGIN/./lib" ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/"$APP_NAME"
chrpath -r "\$ORIGIN/./lib" ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/"$APP_NAME"_console
rm -rf ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/share
cp logo.ico ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/"$APP_NAME".ico
mkdir -p ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"
mkdir -p ./dpkg/Linux_"$APP_VERSION"_x86_64/DEBIAN
cp ./dpkg/DEBIAN/control ./dpkg/Linux_"$APP_VERSION"_x86_64/DEBIAN/control
# 配置打包信息
sed -i "s/#VERSION#/$MAJARVERSION.$SUBVERSION$REVISION/g" ./dpkg/Linux_"$APP_VERSION"_x86_64/DEBIAN/control
cd ./dpkg/Linux_"$APP_VERSION"_x86_64 || exit
SIZE=$(du -sh -B 1024 ./ | sed "s/.\///g")
cd -
InstalledSize=$SIZE
sed -i "s/#SIZE#/$InstalledSize/g" ./dpkg/Linux_"$APP_VERSION"_x86_64/DEBIAN/control
chmod 755 ./dpkg/Linux_"$APP_VERSION"_x86_64/* -R
mkdir -p ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME"/plugins/"$APP_NAME"
# 打包
dpkg -b ./dpkg/Linux_"$APP_VERSION"_x86_64 ./dpkg/nuclei-"$APP_NAME"-"$APP_VERSION"-linux-x64.deb
echo build success!
# 压缩
echo "wait taring..."
cp -r ./dpkg/Linux_"$APP_VERSION"_x86_64/opt/"$APP_NAME" ./dpkg/"$APP_NAME"
cd dpkg
tar -zcvf nuclei-"$APP_NAME"-"$APP_VERSION"-linux-x64.tar.gz "$APP_NAME"
cd ..
rm -rf ./dpkg/"$APP_NAME"
echo "tar finish!"
###############################################################################
