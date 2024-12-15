# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(DS_VERSION "1.99.0" CACHE STRING "Define project version")
project(OCEANShell
    VERSION "${DS_VERSION}"
    DESCRIPTION "ocean-shell"
    HOMEPAGE_URL "https://github.com/lingmoos/ocean-shell"
    LANGUAGES CXX C
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_WITH_X11 "Build X11 emulation" ON)

set(DS_BUILD_WITH_QT6 ON CACHE BOOL "Build ocean-shell with Qt6")

if (DS_BUILD_WITH_QT6)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
    set(DTK_VERSION_MAJOR "")
endif()

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Library install path")
set(OCEAN_SHELL_PACKAGE_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/ocean-shell CACHE STRING "Package install path")
set(OCEAN_SHELL_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/ocean-shell CACHE STRING "Plugin install path")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/ocean-shell" CACHE STRING "Headers install path")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/OCEANShell" CACHE STRING "CMake config file install directory")
set(QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/qml" CACHE STRING "Qml plugin install directory")
set(OCEAN_SHELL_TRANSLATION_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/ocean-shell" CACHE STRING "Translation install directory")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};${ECM_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
include(OCEANShellPackageMacros)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Concurrent Quick WaylandClient DBus LinguistTools Sql)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)
find_package(WaylandProtocols REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(frame)
add_subdirectory(shell)
add_subdirectory(example)
add_subdirectory(panels)
add_subdirectory(applets)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif(BUILD_TESTING)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/misc/OCEANShellConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/OCEANShellConfig.cmake"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/OCEANShellConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
# Install cmake config file
install(FILES "${PROJECT_SOURCE_DIR}/cmake/OCEANShellPackageMacros.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/OCEANShellConfig.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/OCEANShellConfigVersion.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
