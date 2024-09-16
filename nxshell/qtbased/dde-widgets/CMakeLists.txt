# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(VERSION 3.7)

project(dde-widgets)

set(CMAKE_CXX_STANDARD 17)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

add_definitions(-DDefaultPluginDirectory="${CMAKE_INSTALL_FULL_LIBDIR}/dde-widgets/plugins")

include_directories(
    ${PROJECT_SOURCE_DIR}/app/utils/dbus/xml2cpp/types/
    ${PROJECT_SOURCE_DIR}/app/utils/dbus/xml2cpp/
)

add_subdirectory(interface)
add_subdirectory(app)
add_subdirectory(tests)

# buildin widgets.
add_subdirectory(notification)
add_subdirectory(memorymonitor)
add_subdirectory(worldclock)
add_subdirectory(calendar)

# Only build example when dde-widgets-dev in installed.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(DdeWidgets QUIET)
    if (DdeWidgets_FOUND)
        add_subdirectory(example)
    endif()
endif()

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dde-widgets/translations)

