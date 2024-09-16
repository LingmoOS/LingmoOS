# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "shutdown")

project(${PLUGIN_NAME})

# Sources files
file(GLOB_RECURSE SRCS
    "*.h"
    "*.cpp"
    "../widgets/*.h"
    "../widgets/*.cpp"
    "../util/imageutil.h"
    "../util/imageutil.cpp"
    "../common/commoniconbutton.h"
    "../common/commoniconbutton.cpp"
)

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

#if (${CMAKE_SYSTEM_PROCESSOR}  STREQUAL "aarch64")
#    add_definitions("-DDISABLE_POWER_OPTIONS")
#endif()
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS})
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS}
                                                 ${Qt5DBus_INCLUDE_DIRS}
                                                 ${QGSettings_INCLUDE_DIRS}
                                                 ../../../interfaces
                                                 ../common
                                                 ../util
                                                 ../widgets)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
install(FILES "resources/shutdown.svg" DESTINATION share/dde-dock/icons/dcc-setting)
