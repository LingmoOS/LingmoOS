# SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

install(FILES
    protocols/appmenu.xml
    protocols/blur.xml
    protocols/client-management.xml
    protocols/contrast.xml
    protocols/dpms.xml
    protocols/dde-restrict.xml
    protocols/dde-seat.xml
    protocols/dde-security.xml
    protocols/dde-shell.xml
    protocols/fake-input.xml
    protocols/fullscreen-shell.xml
    protocols/idle.xml
    protocols/keystate.xml
    protocols/outputdevice.xml
    protocols/output-management.xml
    protocols/kde-lockscreen-overlay-v1.xml
    protocols/kde-primary-output-v1.xml
    protocols/kde-output-device-v2.xml
    protocols/kde-output-management-v2.xml
    protocols/plasma-shell.xml
    protocols/plasma-virtual-desktop.xml
    protocols/plasma-window-management.xml
    protocols/remote-access.xml
    protocols/server-decoration-palette.xml
    protocols/server-decoration.xml
    protocols/shadow.xml
    protocols/slide.xml
    protocols/strut.xml
    protocols/surface-extension.xml
    protocols/text-input-unstable-v2.xml
    protocols/text-input.xml
    protocols/wayland-eglstream-controller.xml
    protocols/wlr-data-control-unstable-v1.xml
    protocols/zkde-screencast-unstable-v1.xml
    protocols/kde-output-order-v1.xml
    protocols/dde-kvm.xml

    DESTINATION ${KDE_INSTALL_DATADIR}/deepin-wayland-protocols)

# Backward compatibility for previously used non-standard protocol file names
# TODO KF6 remove
install(FILES protocols/zkde-screencast-unstable-v1.xml RENAME screencast.xml DESTINATION ${KDE_INSTALL_DATADIR}/deepin-wayland-protocols)
