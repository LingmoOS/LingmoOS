# SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

add_library(effectsplugin
    expoarea.cpp
    expolayout.cpp
    plugin.cpp
)

target_link_libraries(effectsplugin
    kwineffects

    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/private/effects)
install(TARGETS effectsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/private/effects)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/private/effects)
