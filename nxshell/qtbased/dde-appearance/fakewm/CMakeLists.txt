# SPDX-FileCopyrightText: None
#
# SPDX-License-Identifier: CC0-1.0

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Concurrent Gui)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(Dtk REQUIRED COMPONENTS Core)

pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)

set(SRCS
  ./dbus/deepinwmfaker.h
  ./dbus/deepinwmfaker.cpp
  main.cpp
)

qt5_add_dbus_adaptor(
    wm_SRCS
    ../dbus/com.deepin.wm.xml
    ./dbus/deepinwmfaker.h
    DeepinWMFaker
)

add_executable(dde-fakewm
    ${SRCS}
    ${wm_SRCS}
)

target_link_libraries(dde-fakewm
    Qt5::Core
    Qt5::DBus
    Qt5::Concurrent
    Qt5::Gui
    KF5::ConfigCore
    KF5::WindowSystem
    KF5::GlobalAccel
    PkgConfig::QGSettings
    Dtk::Core
)

install(TARGETS dde-fakewm DESTINATION ${CMAKE_INSTALL_BINDIR})
