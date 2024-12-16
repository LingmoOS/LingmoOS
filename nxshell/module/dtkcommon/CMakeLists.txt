# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)
set (DTK_VERSION "5.6.12" CACHE STRING "define project version")
project(dtkcommon
  VERSION ${DTK_VERSION}
  DESCRIPTION "DTK common"
  HOMEPAGE_URL "https://github.com/lingmoos/dtkcommon"
)
message(STATUS "Project version: ${PROJECT_VERSION}")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)
install(DIRECTORY configs DESTINATION "share/dsg" FILES_MATCHING PATTERN "*")

set(VERSION_SUFFIX)
configure_file(
  "${CMAKE_SOURCE_DIR}/misc/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/DtkConfig.cmake"
  @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DtkConfig.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk")
set(VERSION_SUFFIX 6)
configure_file(
  "${CMAKE_SOURCE_DIR}/misc/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Dtk6Config.cmake"
  @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Dtk6Config.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk6")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DtkBuildHelperConfigVersion.cmake"
    VERSION ${DTK_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DtkBuildHelperConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkBuildHelper")

install(FILES "${CMAKE_SOURCE_DIR}/cmake/DtkBuildHelper.cmake"
    RENAME "DtkBuildHelperConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkBuildHelper")
