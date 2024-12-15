# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE INCLUDE_PRIVATE_FILES
    "../include/daudioport_p.h"
    "../include/daudiocard_p.h"
    "../include/daudiomanager_p.h"
    "../include/daudiodevice_p.h"
    "../include/daudiostream_p.h"
    "../include/daudiofactory_p.h"
    "../include/daudioplugin_p.h"
)
file(GLOB_RECURSE INCLUDE_FILES
    "../include/DAudioPort"
    "../include/DAudioCard"
    "../include/DAudioManager"
    "../include/DAudioDevice"
    "../include/DAudioStream"
    "../include/dtkaudiomanager_global.h"
    "../include/daudioport.h"
    "../include/daudiocard.h"
    "../include/daudiomanager.h"
    "../include/daudiodevice.h"
    "../include/daudiostream.h"
)
file(GLOB_RECURSE SRCS
    "./daudioport.cpp"
    "./daudiocard.cpp"
    "./daudiofactory.cpp"
    "./daudioplugin.cpp"
    "./daudiomanager.cpp"
    "./daudiodevice.cpp"
    "./daudiostream.cpp"
    "./dbus/*.h"
    "./dbus/*.cpp"
)

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${INCLUDE_PRIVATE_FILES}
    ${SRCS}
)

include_directories(../include)

add_definitions(-DLIBDTKAUDIOMANAGER_LIBRARY)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkCore_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_LIBRARIES}
)

install(FILES ${INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
