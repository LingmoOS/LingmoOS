# SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.5)

project(DeepinWaylandProtocols VERSION 1.10.0)

include(FeatureSummary)
include(CMakePackageConfigHelpers)

# ECM setup
find_package(ECM 5.69.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)
include(KDEInstallDirs)
include(ECMSetupVersion)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/DeepinWaylandProtocols")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/DeepinWaylandProtocolsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/DeepinWaylandProtocolsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS KDE_INSTALL_DATADIR
                              )
ecm_setup_version(PROJECT VARIABLE_PREFIX DEEPIN_WAYLAND_PROTOCOLS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/deepin_wayland_protocols_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/DeepinWaylandProtocolsConfigVersion.cmake"
                        SOVERSION 5)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/DeepinWaylandProtocolsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/DeepinWaylandProtocolsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
