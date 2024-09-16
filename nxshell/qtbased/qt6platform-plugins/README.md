# qt5platform-plugins

qt5platform-plugins is the Qt platform integration plugin for Deepin Desktop Environment.

## Dependencies

### Build dependencies

* pkg-config
* mtdev
* xcb-xkb
* xcb-render-util
* xcb-image
* xcb-icccm4
* xcb-keysyms1-dev
* egl1-mesa
* xkbcommon-x11
* dbus-1
* udev
* xrender
* xi
* sm
* xcb-xinerama
* fontconfig
* freetype6
* glib2.0
* xcb-damage
* xcb-composite
* cairo2
* Qt5 (>= 5.6)
  * Qt5-Core
  * Qt5-Gui
  * Qt5-OpenGL
  * Qt5-X11extras
  * Qt5-Core-Private

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:

Support disabling some modules, add CONFIG+=<val> when executing qmake.

val:

- DISABLE_WAYLAND
- DISABLE_XCB

```
mkdir build
cd build
qmake ..
make
```

3. Install:
```
$ sudo make install
```

## Usage

To be done.

## Getting help

You may also find these channels useful if you encounter any other issues:

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

qt5platform-plugins is licensed under [LGPL-3.0-or-later](LICENSE).

## Environment variable

* DXCB_PRINT_WINDOW_CREATE: Print the information of the QWindow when the local window is created
* DXCB_PAINTENGINE_DISABLE_FEATURES: Specify the qpaintengine:: paintenginefeatures to disable, which is only valid for the QPainter drawing system
* DXCB_FAKE_PLATFORM_NAME_XCB: Force the value of qguiapplication:: platformname property to be "xcb"
* DXCB_DISABLE_HOOK_CURSOR: It is prohibited to use the value of qwindow:: devicepixelratio to automatically scale the size of the cursor on this window
* DXCB_REDIRECT_CONTENT: Set whether to allow XDamage to redirect the content drawn in the window with dxcb mode enabled. "true" indicates that it is allowed, and "false" indicates that it is not allowed. Otherwise, the value set by the window itself will be used. If the window does not set any value, the redirection mode will be enabled when the surface type of the window is QSurface:: OpenGLSurface. Otherwise, it will not be enabled
* DXCB_REDIRECT_CONTENT_WITH_NO_COMPOSITE: It is mandatory to allow the content drawn in the XDamage redirected window in the mode without window effects. If it is not set or the value is empty, the content drawn in the XDamage redirected window will not be used if the window manager does not support Composite
