# SPDX-FileCopyrightText: 2019 Aleix Pol <apol@kde.org>
# SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
# SPDX-License-Identifier: GPL-2.0-or-later

configure_file(startlingmomobile.in ${CMAKE_CURRENT_BINARY_DIR}/startlingmomobile)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startlingmomobile DESTINATION ${KDE_INSTALL_BINDIR})

configure_file(lingmo-mobile.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmo-mobile.desktop)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-mobile.desktop
        DESTINATION ${KDE_INSTALL_DATADIR}/wayland-sessions
)

# For Lingmo developers
configure_file(startlingmomobile-dev.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/startlingmomobile-dev.sh)
configure_file(lingmo-mobile-dev.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmo-mobile-dev.desktop)
configure_file(install-sessions.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/install-sessions.sh)
