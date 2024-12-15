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
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Svg DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)

#if (${CMAKE_SYSTEM_PROCESSOR}  STREQUAL "aarch64")
#    add_definitions("-DDISABLE_POWER_OPTIONS")
#endif()

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS})
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ../../../interfaces
    ../common
    ../util
    ../widgets
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${XCB_EWMH_LIBRARIES}
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins)
install(FILES "resources/oceanui-shutdown.dci" DESTINATION share/ocean-dock/icons/oceanui-setting)

dtk_add_config_meta_files(APPID org.lingmo.ocean.dock FILES ../configs/org.lingmo.ocean.dock.plugin.shutdown.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ocean.tray-loader FILES ../configs/org.lingmo.ocean.dock.plugin.shutdown.json)
