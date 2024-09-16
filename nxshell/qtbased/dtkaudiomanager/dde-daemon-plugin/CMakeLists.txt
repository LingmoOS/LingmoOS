# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtkaudiomanager-ddedaemon)

file(GLOB_RECURSE INCLUDE_PRIVATE_FILES
    "../include/daudioport_p.h"
    "../include/daudiocard_p.h"
    "../include/daudioplugin_p.h"
)
file(GLOB_RECURSE INCLUDE_FILES
    "./daudioport_daemon.h"
    "./daudiocard_daemon.h"
    "./daudioplugin_daemon.h"
    "./daudiomanager_daemon.h"
    "./daudiodevice_daemon.h"
    "./daudiostream_daemon.h"
    "./daemonhelpers.hpp"
)
file(GLOB_RECURSE SRCS
    "./daudioport_daemon.cpp"
    "./daudiocard_daemon.cpp"
    "./daudioplugin_daemon.cpp"
    "./daudiomanager_daemon.cpp"
    "./daudiodevice_daemon.cpp"
    "./daudiostream_daemon.cpp"
    "../src/dbus/*.h"
    "../src/dbus/*.cpp"
)

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${INCLUDE_PRIVATE_FILES}
    ${SRCS}
)

include_directories(../include)
include_directories(../src)

SET(LIBRARY_OUTPUT_PATH ../plugins)
set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
)

install(TARGETS ${BIN_NAME} DESTINATION ${DAUDIO_MANAGER_PLUGIN_DIR})
