## 编译说明

### 编译步骤

可以选择通过Qt Creator打开工程文件，然后点击构建按钮进行编译调试；也可以使用项目中预置的编译脚本进行编译，使用编译脚本可以直接编译输出打包好的安装包文件。

- windows（mingw）

    修改build_setup.bat中QT_DIR、QT_TOOLS_DIR、INNO_SETUP_DIR变量的值，然后运行build_setup.bat脚本即可。

- windows（msvc）

    修改build_setup_msvc.bat中QT_DIR、QT_TOOLS_DIR、INNO_SETUP_DIR变量的值，然后运行build_setup.bat脚本即可。

- linux

    修改build_deb.sh中QT_DIR变量的值，然后运行build_deb.sh脚本即可。

- macos

    修改build_dmg.sh中QT_DIR变量的值，然后运行build_dmg.sh脚本即可。
