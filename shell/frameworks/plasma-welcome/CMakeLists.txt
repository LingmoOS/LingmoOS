# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
# SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

project(welcome)
set(PROJECT_VERSION "6.1.80")

cmake_minimum_required(VERSION 3.16)

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEClangFormat)
include(KDEGitCommitHooks)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(ECMQmlModule)
include(ECMDeprecationSettings)

set(DISTRO_CUSTOM_PAGE_FOLDER "/usr/share/plasma/plasma-welcome/extra-pages/" CACHE STRING "Folder on disk to find custom pages")
set(DISTRO_CUSTOM_INTRO_FILE "/usr/share/plasma/plasma-welcome/intro-customization.desktop" CACHE STRING "File specifying custom intro content")

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.hpp *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX PLASMA_WELCOME
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/plasma-welcome-version.h"
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Network Qml QuickControls2 Svg)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS Kirigami Config CoreAddons ConfigWidgets DBusAddons I18n KIO NewStuff Service WindowSystem DBusAddons KCMUtils Svg JobWidgets)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)
find_package(KF6KirigamiAddons 1.2.0 REQUIRED)

find_package(KF6UserFeedback)
set_package_properties(KUserFeedback PROPERTIES
    DESCRIPTION "Framework for collecting user feedback for apps via telemetry and surveys"
    PURPOSE "Required for embedded User Feedback settings page"
    TYPE RUNTIME
)

ecm_set_disabled_deprecation_versions(QT 6.5
    KF 5.240
)

add_subdirectory(src)

install(PROGRAMS org.kde.plasma-welcome.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.plasma-welcome.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

ki18n_install(po)
