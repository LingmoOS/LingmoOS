# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(lingmo-system-monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)

add_library(monitor-backend STATIC
    src/diskreader.cpp
    src/fileutil.cpp
    src/gpureader.cpp
    src/monitor.cpp
    src/networkreader.cpp
    src/preferences.cpp
    src/processlist.cpp
    src/procreader.cpp
    src/textutil.cpp
)
target_include_directories(monitor-backend PUBLIC src)
target_link_libraries(monitor-backend PUBLIC Lingmo::SDKBase PRIVATE PkgConfig::ICU)

# The glyphs, built in.
file(GLOB SYSTEM_MONITOR_ICONS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/icons/*.svg)
set(SYSTEM_MONITOR_EMBEDDED ${CMAKE_CURRENT_BINARY_DIR}/icons.cpp)
# A list would be split into separate arguments on the way to the script.
string(REPLACE ";" "|" SYSTEM_MONITOR_ICON_LIST "${SYSTEM_MONITOR_ICONS}")
add_custom_command(OUTPUT ${SYSTEM_MONITOR_EMBEDDED}
    COMMAND ${CMAKE_COMMAND} -DOUTPUT=${SYSTEM_MONITOR_EMBEDDED}
            -DFILES=${SYSTEM_MONITOR_ICON_LIST} -P ${PROJECT_SOURCE_DIR}/cmake/EmbedIcons.cmake
    DEPENDS ${SYSTEM_MONITOR_ICONS} ${PROJECT_SOURCE_DIR}/cmake/EmbedIcons.cmake
    VERBATIM)

add_library(monitor-ui STATIC
    src/views/chartcard.cpp
    src/views/colors.cpp
    src/views/components.cpp
    src/views/historygraph.cpp
    src/views/mainwindow.cpp
    src/views/processdetails.cpp
    src/views/processespage.cpp
    src/views/resourcepage.cpp
    src/views/summarycard.cpp
    src/views/textmetrics.cpp
    ${SYSTEM_MONITOR_EMBEDDED}
)
target_include_directories(monitor-ui PUBLIC src src/views)
target_link_libraries(monitor-ui PUBLIC monitor-backend Lingmo::SDK)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE SYSTEM_MONITOR_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE monitor-ui)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

lingmo_add_translations(lingmo-system-monitor SOURCES src DESKTOP_FILES lingmo-system-monitor.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-system-monitor.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
