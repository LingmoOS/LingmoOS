# SPDX-FileCopyrightText: 2024 LingmoSoft Co., Ltd.
# SPDX-FileCopyrightText: 2024 Hongfei Shang
#
# SPDX-License-Identifier: MulanPSL-2.0

cmake_minimum_required(VERSION 3.5)

project(LingmoWaylandProtocols VERSION 1.0.0)

include(FeatureSummary)
include(GNUInstallDirs)

add_subdirectory(src)

set(PROJECT_NAME LingmoWaylandProtocols)
set(CMAKE_PACKAGE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
set(INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}
    CACHE PATH "Location of data files" )

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/LingmoWaylandProtocolsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoWaylandProtocolsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_PACKAGE_DIR}
        PATH_VARS INSTALL_DATADIR
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/LingmoWaylandProtocolsConfigVersion.cmake
  VERSION ${CMAKE_PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LingmoWaylandProtocolsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoWaylandProtocolsConfigVersion.cmake
        DESTINATION ${CMAKE_PACKAGE_DIR}
        COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
