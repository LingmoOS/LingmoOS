# Deepin Wayland Protocols

This project should be installing only the xml files of the non-standard wayland
protocols we use in Deepin.

They are installed to $PREFIX/share/deepin-wayland-protocols.

## Usage
You can get the directory where they're installed by using

    find_package(DeepinWaylandProtocols)

Then they can be accessed using `${DEEPIN_WAYLAND_PROTOCOLS_DIR}`.

You can learn more about such protocol files in
https://wayland.freedesktop.org/docs/html/ch04.html.
