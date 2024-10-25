cmake_minimum_required(VERSION 3.16)

project(lingmo-%{APPNAMELC})

find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(KF6 REQUIRED COMPONENTS
    Lingmo
    I18n
)

find_package(Qt6 CONFIG REQUIRED COMPONENTS
    Qml
    Gui
    Core
)

# wallpaper applet
# TODO: adapt "org.kde.lingmo" here & elsewhere if needed (see README)
lingmo_install_package(package org.kde.lingmo.%{APPNAMELC} wallpapers wallpaper)

# qml extension plugin
add_subdirectory(plugin)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
