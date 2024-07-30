# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

cmake_minimum_required(VERSION 3.16)
#-----------------------------------------------------------------------------
#                           GENERAL CONFIGURATION
#-----------------------------------------------------------------------------
project(plasma-firewall)
set(PROJECT_VERSION "6.1.80")

set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT_MIN_VERSION "6.6.0")

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-----------------------------------------------------------------------------
#                                OPTIONS
#-----------------------------------------------------------------------------

include(FeatureSummary)

option(BUILD_FIREWALLD_BACKEND "Build the firewalld backend" ON)
add_feature_info(FIREWALLD_BACKEND ${BUILD_FIREWALLD_BACKEND} "Build the FirewallD backend")

option(BUILD_UFW_BACKEND "Build the Uncomplicated Firewall (UFW) backend" ON)
add_feature_info(UFW_BACKEND ${BUILD_UFW_BACKEND} "Build the Uncomplicated Firewall (UFW) backend")

#-----------------------------------------------------------------------------
#                              DEPENDENCIES
#-----------------------------------------------------------------------------
find_package(Python3 COMPONENTS Interpreter)
# KF6
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMDeprecationSettings)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Qml
    Quick
    DBus
    Test
)

ecm_set_disabled_deprecation_versions(QT 6.4
    KF 5.240
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS 
    Config
    CoreAddons
    KCMUtils
    I18n
    Auth
)


#-----------------------------------------------------------------------------
#                               SOURCES
#-----------------------------------------------------------------------------
add_subdirectory(kcm)

if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

#-----------------------------------------------------------------------------
#                               Clang Format
#-----------------------------------------------------------------------------
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

install(FILES org.kde.plasma.firewall.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

#-----------------------------------------------------------------------------
#                                Translations
#-----------------------------------------------------------------------------
ki18n_install(po)

#-----------------------------------------------------------------------------
#                               BUILD SUMMARY
#-----------------------------------------------------------------------------
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
