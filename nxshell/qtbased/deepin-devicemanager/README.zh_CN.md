## deepin-devicemanager
设备管理器是查看、管理硬件设备的工具软件。

## 依赖
The GUI is based on [DTK](https://github.com/linuxdeepin/dtkwidget), Qt (Supported >= 5.12).

Build-Depends: debhelper (>= 11), pkg-config, qt5-qmake, qt5-default, libdtkwidget-dev, libdtkgui-dev, qtbase5-private-dev, libdframeworkdbus-dev, libcups2-dev

Execute-Depends: libc6 (>= 2.14), libdframeworkdbus2, libdtkcore5 (>= 5.0.9+r2+g0dc0efd), libdtkgui5 (>= 5.0.9+r0+g773b21d), libdtkwidget5, libgcc1 (>= 1:3.0), libgl1, libqt5core5a (>= 5.11.0~rc1), libqt5dbus5 (>= 5.0.2), libqt5gui5 (>= 5.8.0), libqt5svg5 (>= 5.6.0~beta), libqt5widgets5 (>= 5.11.0~rc1), libqt5xml5 (>= 5.0.2), libstdc++6 (>= 6), qtbase-abi-5-11-3, libcups2 (>= 1.7.0), smartmontools, dmidecode[i386 arm64 amd64], x11-xserver-utils, hwinfo, upower, deepin-shortcut-viewer, lshw, cups

## 链接
- [linux-hardware.org](https://linux-hardware.org/)

## 运行依赖
- [dmidecode](http://www.nongnu.org/dmidecode/)
- [lshw](https://ezix.org/project/wiki/HardwareLiSter)
- [lspci](https://github.com/linuxhw/LsPCI)
- lsb_release
- hciconfig
- bluetoothctl
- [hwinfo](https://github.com/linuxhw/HWInfo)
- lsusb
- lscpu
- [smartctl](https://www.smartmontools.org/)
- [xrandr](https://www.x.org/wiki/Projects/XRandR/)
- [CUPS](https://www.cups.org/index.html)
- [upower](https://upower.freedesktop.org/)
- deepin-shortcut-viewer

## 第三方库 (source code)
- [docx](https://github.com/lpxxn/docx)
- [QtXlsx](http://qtxlsx.debao.me)

## 安装
sudo apt-get install deepin-devicemanager

## 编译构建
- mkdir build
- cd build
- qmake ..
- make

## 用途
Caution: deepin-devicemanager is depend on polkit, So if build from source code, 
you have to add policy to usr/share/polkit-1/actions/. 
add deepin-devicemanager-authenticateProxy to /usr/bin.
If run in Qt Creator, you should set [Project][Run Settings] [Run] [Run configuration] to src from 2 projects authenticateProxy and src.

- sudo cp authenticateProxy/policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy /usr/share/polkit-1/actions/
- sudo cp build-all-unknown-Debug/authenticateProxy/deepin-devicemanager-authenticateProxy /usr/bin

- ./deepin-devicemanager

## 开源许可证
deepin-devicemanager 根据 [GPL-3.0-or-later]（许可证）获得许可.
