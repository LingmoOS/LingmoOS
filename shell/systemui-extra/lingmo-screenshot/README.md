<div align="center">
  <p>
    <h1>
      Lingmo-Screenshot
    </h1>
    <h4>Powerful yet simple to use screenshot software forked from flameshot.</h4>
  </p>
</div>
  lingmo-screenshot is a powerful screenshot and screen recording tool. It allows you to take screenshots of the captured images in rectangular and circular boxes, set blur, add markers, add text and more. And set mouse display, sound recording, video frame rate, audio bit rate when making screen recording.


## Index

- [Features](#features)
- [Usage](#usage)
  - [Dependencies](#dependencies)
  - [Installations](installations)
  - [Terminal_Commond](terminal-commond)
  - [Keyboard Shortcuts](#keyboard-shortcuts)

## Features

- Customizable appearance.
- Easy to use.
- In-app screenshot  and  screenrecorder edition.
- DBus interface.

## Usage
### Dependencies
You will need the following packages to compile lingmo-screenshot:
-	            qt5-qmake,
-               qtbase5-dev,
-               qttools5-dev-tools,
-               libqt5svg5-dev,
-               libavformat-dev,
-               libavcodec-dev,
-               libavutil-dev,
-               libswscale-dev,
-               libpulse-dev,
-               libqt5x11extras5,
-               libqt5x11extras5-dev,
-               libxfixes3,
-               libxfixes-dev,
-               libx11-dev,
-               libxinerama1,
-               libxinerama-dev,
-               libgsettings-qt-dev,
-               libkf5windowsystem5
-               libkf5windowsystem-dev,
-               libxtst-dev,
You can Build and Install by apt command
```shell
sudo mk-build-deps -i lingmo-screenshot/debian/control
```
###Installations
After installing all the dependencies, finally run the following commands in the sources root directory:
```shell
git clone https://github.com/ubuntulingmo/lingmo-screenshot.git
cd lingmo-screenshot
debuild
cd ..
dpkg -i lingmo-screenshot_1.0.0-1_amd64.deb
```

### Terminal Command
You can use the graphical menu to configure Lingmo-ScreenShot, but alternatively you can use your terminal or scripts to do so.

- Open the configuration menu:

    ```shell
    lingmo-screenshot config
    ```

- Show the initial help message in the capture mode:

    ```shell
    lingmo-screenshot config --showhelp true
    ```

- For more information about the available options use the help flag:

    ```shell
    lingmo-screenshot config -h
    ```
### Keyboard Shortcut
These shortcuts are available in GUI mode:

|  Keys                                                                     |  Description                                                   |
|---                                                                        |---                                                             |
| <kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd>                    | Move selection 1px                                             |
| <kbd>Shift</kbd> + <kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd> | Resize selection 1px                                           |
| <kbd>Esc</kbd>                                                            | Quit capture                                                   |
| <kbd>Ctrl</kbd> + <kbd>C</kbd>                                            | Copy to clipboard                                              |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                                            | Save selection as a file                                       |
| <kbd>Ctrl</kbd> + <kbd>Z</kbd>                                            | Undo the last modification                                     |
