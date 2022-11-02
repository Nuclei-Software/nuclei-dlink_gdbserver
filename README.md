# Introduction

![](img\dlink_gdbserver.png)

# Qt In GUI Mode

```bash
qtcreator dlink_gdbserver.pro &
```

## Compile

![](img\gui_compile.png)

## Run

![](img\gui_run.png)

# Qt In Command Line Mode

## Compile

```bash
cd dlink_gdbserver

qmake dlink_gdbserver.pro -o ~/build/

make --directory=~/build/
```

## Run

```bash
cd build

# Starting In GUI Mode
./dlink_gdbserver

# Starting In Command Line Mode
./dlink_gdbserver -f ../dlink_gdbserver.cfg
```

# 
