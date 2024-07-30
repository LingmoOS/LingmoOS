# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "1.2.2")

project(plasma-plasmapass VERSION ${PROJECT_VERSION})

set(PROJECT_VERSION_MAJOR 1)

set(QT_MIN_VERSION "5.15")
set(KF5_MIN_VERSION "5.90.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)

find_package(OATH)
set_package_properties(OATH
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Provides components to build one-time password authentication systems"
    PURPOSE "Generates OTP from secret stored in password files")

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
else()
    find_package(QGpgme ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
endif()
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()
find_package(KF${KF_MAJOR_VERSION} ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    ItemModels
    KIO
)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Plasma REQUIRED)
    find_package(Plasma5Support REQUIRED)
else()
    find_package(KF5Plasma REQUIRED)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Gui
    Qml
    Concurrent
)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DQT_NO_FOREACH)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# plasmoid
plasma_install_package(package org.kde.plasma.pass)

# qml extension plugin
add_subdirectory(plugin)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
install(FILES plasma-pass.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

