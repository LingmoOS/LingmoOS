# Deepin Screen Recorder

This is default screen recorder application for Deepin.

## Dependencies

In debian, use below command to install compile dependencies:

libc6 (>= 2.14), libgcc1 (>= 1:3.0), libgl1-mesa-glx | libgl1, libqt5core5a (>= 5.6.0~beta), libqt5dbus5 (>= 5.0.2), libqt5gui5 (>= 5.2.0), libqt5network5 (>= 5.0.2), libqt5widgets5 (>= 5.0.2), libqt5x11extras5 (>= 5.6.0), libstdc++6 (>= 4.1.1), libx11-6, libxcb-util0 (>= 0.3.8), libxcb1, libxext6, libxtst6

## Installation

* mkdir build
* cd build
* qmake ..
* make
* ./deepin-screen-recorder

## Usage

1. Select area need to record
2. Then click "record" button to record
3. Click tray icon to stop record, and save file on desktop

Below is keymap list for deepin-screen-recorder:

| Function					      | Shortcut                                         |
|---------------------------------|--------------------------------------------------|
| Cancel                          | **ESC**                                          |
| Adjust position of select area  | **Up**, **Down**, **Left**, **Right**            |
| Adjust size of select area      | **Ctrl** + **Up**, **Down**, **Left**, **Right** |
| Stop record                     | Same keystroke that start deepin-screen-recorder |


## Config file
Configure file save at:
~/.config/deepin/deepin-screen-recorder/config.conf

You can change default format for save file. 

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Developers).

## License

Deepin Screen Recorder is licensed under [GPL-3.0-or-later](LICENSE).
