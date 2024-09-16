# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required( VERSION 3.1.0 )
set (PROJECT_NAME deepin-screen-recorder)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Test Widgets Network DBus Sql PrintSupport Svg Concurrent LinguistTools Multimedia MultimediaWidgets X11Extras)

project( ${PROJECT_NAME} )

set(LINK_DIR X11 Xext Xtst Xfixes Xcursor procps)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 CONFIG REQUIRED ${QtModule})

set ( deepin-screen-recorder_HDRS
    constant.h
    keydefine.h
    utils/baseutils.h
    utils/calculaterect.h
    utils/saveutils.h
    utils/shapesutils.h
    utils/dbusutils.h
    utils/desktopinfo.h
    RecorderRegionShow.h
    main_window.h
    record_process.h
    settings.h
    utils.h
    record_button.h
    record_option_panel.h
    countdown_tooltip.h
    event_monitor.h
    button_feedback.h
    process_tree.h
    camera_process.h
    show_buttons.h
    utils/audioutils.h
    menucontroller/menucontroller.h
    utils/configsettings.h
    utils/shortcut.h
    utils/tempfile.h
    widgets/savetips.h
    widgets/fontsizewidget.h
    widgets/colorbutton.h
    widgets/zoomIndicator.h
    widgets/majtoolbar.h
    widgets/textedit.h
    widgets/bigcolorbutton.h
    widgets/toptips.h
    widgets/toolbar.h
    widgets/savebutton.h
    widgets/textbutton.h
    widgets/shapeswidget.h
    widgets/toolbutton.h
    widgets/maintoolwidget.h
    widgets/subtoolwidget.h
    widgets/keybuttonwidget.h
    widgets/sidebar.h
    widgets/shottoolwidget.h
    widgets/colortoolwidget.h
    dbusinterface/dbuszone.h
    dbusinterface/dbuscontrolcenter.h
    dbusinterface/dbusnotify.h
    dbusinterface/dbussoundeffect.h
    dbusservice/dbusscreenshotservice.h
    dbusservice/dbusscreenshot.h
    widgets/camerawidget.h
    screenshot.h
    utils/voicevolumewatcher.h
    utils/camerawatcher.h
    widgets/tooltips.h
    widgets/filter.h
    utils/screengrabber.h
    dbusinterface/drawinterface.h
    screen_shot_event.h
    lib/GifH/gif.h
    xgifrecord.h
    recordertablet.h
    )

set ( deepin-screen-recorder_SRCS
    main.cpp
    main_window.cpp
    record_process.cpp
    settings.cpp
    utils.cpp
    record_button.cpp
    record_option_panel.cpp
    countdown_tooltip.cpp
    constant.cpp
    event_monitor.cpp
    button_feedback.cpp
    process_tree.cpp
    camera_process.cpp
    show_buttons.cpp
    utils/audioutils.cpp
    menucontroller/menucontroller.cpp
    utils/shapesutils.cpp
    utils/tempfile.cpp
    utils/calculaterect.cpp
    utils/shortcut.cpp
    utils/configsettings.cpp
    utils/baseutils.cpp
    widgets/savebutton.cpp
    widgets/toptips.cpp
    widgets/shapeswidget.cpp
    widgets/textbutton.cpp
    widgets/colorbutton.cpp
    widgets/fontsizewidget.cpp
    widgets/textedit.cpp
    widgets/bigcolorbutton.cpp
    widgets/zoomIndicator.cpp
    widgets/majtoolbar.cpp
    widgets/savetips.cpp
    widgets/toolbar.cpp
    widgets/maintoolwidget.cpp
    widgets/subtoolwidget.cpp
    widgets/keybuttonwidget.cpp
    widgets/sidebar.cpp
    widgets/shottoolwidget.cpp
    widgets/colortoolwidget.cpp
    dbusinterface/dbusnotify.cpp
    dbusinterface/dbuszone.cpp
    dbusinterface/dbuscontrolcenter.cpp
    dbusinterface/dbussoundeffect.cpp
    dbusservice/dbusscreenshotservice.cpp
    dbusservice/dbusscreenshot.cpp
    widgets/camerawidget.cpp
    utils/dbusutils.cpp
    screenshot.cpp
    utils/voicevolumewatcher.cpp
    utils/camerawatcher.cpp
    widgets/tooltips.cpp
    widgets/filter.cpp
    utils/desktopinfo.cpp
    utils/screengrabber.cpp
    dbusinterface/drawinterface.cpp
    screen_shot_event.cpp
    RecorderRegionShow.cpp
    xgifrecord.cpp
    recordertablet.cpp
    )

set ( deepin-screen-recorder_RSCS
    ../assets/image/deepin-screen-recorder.qrc
    ../assets/resources/resources.qrc
    ../assets/icons/icons.qrc
    )

add_executable(${PROJECT_NAME}
    ${deepin-screen-recorder_HDRS} ${deepin-screen-recorder_SRCS} ${deepin-screen-recorder_RSCS}
    )

qt5_use_modules(${PROJECT_NAME} ${QtModule})
find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget dtkgui dframeworkdbus xcb xcb-util
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${LINK_DIR} )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/deepin-screen-recorder DESTINATION /usr/share/deepin-manual/manual-assets/application)
