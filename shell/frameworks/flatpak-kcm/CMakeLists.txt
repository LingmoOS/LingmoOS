# SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
# SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

project(flatpakkcm)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DTRANSLATION_DOMAIN="kcm_flatpak")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Flatpak REQUIRED IMPORTED_TARGET flatpak>=0.11.8)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    Svg
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    I18n
    KCMUtils
    ItemModels
)

ecm_set_disabled_deprecation_versions(QT 6.5
    KF 5.240
)

# Can't use regular KDE_INSTALL_FULL_LOCALSTATEDIR because it is wrong in developer setup.
# This variable has to point at a directory which Flapak itself was configured with.
set(LOCALSTATEDIR "/var")
configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_library(kcm_flatpak_static STATIC
    flatpakreference.cpp flatpakreference.h
    flatpakpermission.cpp flatpakpermission.h
    flatpakhelper.cpp flatpakhelper.h
    flatpakcommon.cpp flatpakcommon.h
)
set_property(TARGET kcm_flatpak_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kcm_flatpak_static
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    PkgConfig::Flatpak
    Qt::Core
)

kcmutils_add_qml_kcm(kcm_flatpak SOURCES kcm.cpp kcm.h)

target_link_libraries(kcm_flatpak PRIVATE kcm_flatpak_static)

ki18n_install(po)

add_subdirectory(autotests)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
