# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

project(libdmr VERSION 0.1.0)

set(CMAKE_AUTOMOC ON)
set(CMD_NAME dmr)

add_definitions(-D_LIBDMR_)
remove_definitions(-D_MOVIE_USE_)

pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gstreamer-1.0 glib-2.0)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})

file(GLOB_RECURSE SRCS LIST_DIRECTORIES false *.cpp)
file(GLOB_RECURSE MPV_SRCS LIST_DIRECTORIES false ../backends/mpv/*.cpp)
file(GLOB_RECURSE MEDIAPLAYER_SRCS LIST_DIRECTORIES false ../backends/mediaplayer/*.cpp)
file(GLOB_RECURSE HWDEC_SRCS ../common/hwdec_probe.cpp)
list(APPEND SRCS player_widget.cpp ${MPV_SRCS} ${MEDIAPLAYER_SRCS} ${HWDEC_SRCS})

add_library(${CMD_NAME} SHARED ${SRCS})
set_target_properties(${CMD_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0.1)

target_include_directories(${CMD_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../backends/mediaplayer ${PROJECT_SOURCE_DIR}/../backends/mpv /usr/include/glib-2.0 /usr/include/gstreamer-1.0)

target_link_libraries(${CMD_NAME} PkgConfig::Dtk Qt5::Widgets Qt5::Concurrent
    Qt5::Network Qt5::X11Extras Qt5::Sql Qt5::Svg Qt5::Multimedia Qt5::MultimediaWidgets Qt5::DBus PkgConfig::Gst GL)
#target_link_libraries(${CMD_NAME} PkgConfig::Dtk Qt5::Widgets Qt5::Concurrent
#    Qt5::Network Qt5::X11Extras Qt5::Sql Qt5::DBus PkgConfig::AV GL png jpeg)


configure_file(libdmr.pc.in ${PROJECT_BINARY_DIR}/libdmr.pc @ONLY)

install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES
    player_widget.h
    player_backend.h
    player_engine.h
    playlist_model.h
    movie_configuration.h
    compositing_manager.h
    utils.h
    online_sub.h
    DESTINATION include/libdmr)

install(FILES ${PROJECT_BINARY_DIR}/libdmr.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
