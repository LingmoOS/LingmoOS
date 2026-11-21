# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(lingmo-video-player VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Mpv REQUIRED IMPORTED_TARGET mpv>=2.0)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)

add_library(player-backend STATIC
    src/application.cpp
    src/durationprobe.cpp
    src/languages.cpp
    src/mprisserver.cpp
    src/mpvlibrary.cpp
    src/mpvnode.cpp
    src/player.cpp
    src/playersettings.cpp
    src/playlist.cpp
    src/resumestore.cpp
    src/sleepinhibitor.cpp
    src/softwarevideo.cpp
    src/thumbnailer.cpp
    src/tracks.cpp
    src/urls.cpp
)
target_include_directories(player-backend PUBLIC src)
# libmpv is opened on first use (src/mpvlibrary.cpp); only its headers are needed here.
target_include_directories(player-backend PUBLIC ${Mpv_INCLUDE_DIRS})
target_link_libraries(player-backend PUBLIC Lingmo::SDK ${CMAKE_DL_LIBS} PRIVATE PkgConfig::ICU)

# The glyphs, built in.
file(GLOB VIDEO_PLAYER_ICONS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/icons/*.svg)
set(VIDEO_PLAYER_EMBEDDED ${CMAKE_CURRENT_BINARY_DIR}/icons.cpp)
# A list would be split into separate arguments on the way to the script.
string(REPLACE ";" "|" VIDEO_PLAYER_ICON_LIST "${VIDEO_PLAYER_ICONS}")
add_custom_command(OUTPUT ${VIDEO_PLAYER_EMBEDDED}
    COMMAND ${CMAKE_COMMAND} -DOUTPUT=${VIDEO_PLAYER_EMBEDDED}
            -DFILES=${VIDEO_PLAYER_ICON_LIST} -P ${PROJECT_SOURCE_DIR}/cmake/EmbedIcons.cmake
    DEPENDS ${VIDEO_PLAYER_ICONS} ${PROJECT_SOURCE_DIR}/cmake/EmbedIcons.cmake
    VERBATIM)

add_library(player-ui STATIC
    src/views/controlbar.cpp
    src/views/dialogs.cpp
    src/views/mainwindow.cpp
    src/views/pages.cpp
    src/views/sidepanel.cpp
    src/views/videoview.cpp
    src/views/widgets.cpp
    ${VIDEO_PLAYER_EMBEDDED}
)
target_include_directories(player-ui PUBLIC src src/views)
target_link_libraries(player-ui PUBLIC player-backend Lingmo::SDK)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE VIDEO_PLAYER_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE player-ui)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

lingmo_add_translations(lingmo-video-player SOURCES src DESKTOP_FILES lingmo-video-player.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-video-player.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
