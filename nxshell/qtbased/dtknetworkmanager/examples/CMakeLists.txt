# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_executable(managerdemo managerdemo.cpp)

add_executable(devicedemo devicedemo.cpp)

target_link_libraries(managerdemo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    PkgConfig::Gio
    PkgConfig::NetworkManager
    ${LIB_NAME}
)

target_link_libraries(devicedemo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    PkgConfig::Gio
    PkgConfig::NetworkManager
    ${LIB_NAME}
)
