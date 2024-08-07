@echo off

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 定义Qt目录
set "QT_DIR=C:/Qt/6.5.1/msvc64/bin"
set "QT_TOOLS_DIR=C:/Qt/Tools/msvc64/bin"
:: 定义Inno Setup目录
set "INNO_SETUP_DIR=C:/Program Files (x86)/Inno Setup 6"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 定义APP Name
set "APP_NAME=dlink_gdbserver"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 定义版本号
set /p APP_VERSION=<version.txt
:: 设置环境变量
set "PATH=%QT_DIR%;%QT_TOOLS_DIR%;%INNO_SETUP_DIR%;%PATH%"
:: 编译
del .qmake.stash Makefile
if exist ".\InnoSetup\build" (
    rmdir /Q /S .\InnoSetup\build
)
if exist ".\InnoSetup\plugins" (
    rmdir /Q /S .\InnoSetup\plugins
)
lrelease %APP_NAME%.pro
qmake %APP_NAME%.pro -tp vc
msbuild %APP_NAME%.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildInParallel=false
lrelease %APP_NAME%_console.pro
qmake %APP_NAME%_console.pro -tp vc
msbuild %APP_NAME%_console.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildInParallel=false
:: 配置打包信息
copy /y .\InnoSetup\build_setup.iss .\InnoSetup\build_temp_setup.iss
.\tools\sed\sed.exe -i "s/#VERSION#/%APP_VERSION%/g" .\InnoSetup\build_temp_setup.iss
.\tools\sed\sed.exe -i "s/#VERSIONINFOVERSION#/%APP_VERSION%.000/g" .\InnoSetup\build_temp_setup.iss
del /f /q /a .\sed*
:: 构建打包目录
xcopy /y .\build_debug\out\%APP_NAME%.exe .\InnoSetup\build\
xcopy /y .\build_debug\out\%APP_NAME%_console.exe .\InnoSetup\build\
xcopy /y .\dlink_gdbserver.cfg .\InnoSetup\build\
xcopy /y .\InnoSetup\npk.yml .\InnoSetup\build\
:: 使用windeployqt拷贝依赖dll库到打包目录
windeployqt --dir .\InnoSetup\build .\InnoSetup\build\%APP_NAME%.exe
windeployqt --dir .\InnoSetup\build .\InnoSetup\build\%APP_NAME%_console.exe
xcopy /y .\scripts\Profile.ps1 .\InnoSetup\build\
mkdir ".\InnoSetup\plugins\%APP_NAME%"
mkdir ".\InnoSetup\plugins\%APP_NAME%_console"
:: 打包
echo "wait inno build setup..."
iscc /q ".\InnoSetup\build_temp_setup.iss"
del .\InnoSetup\build_temp_setup.iss
rmdir /Q /S .\build_debug
rmdir /Q /S .\build_release
rmdir /Q /S .\debug
rmdir /Q /S .\release
echo "build success!"
:: 压缩
echo "wait zipping..."
xcopy /y .\InnoSetup\build .\InnoSetup\%APP_NAME% /E /H /C /I
cd InnoSetup
7z a -tzip nuclei-%APP_NAME%-v%APP_VERSION%-win32-x32.zip %APP_NAME%
cd ..
rmdir /Q /S .\InnoSetup\%APP_NAME%
echo "zip finish!"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
