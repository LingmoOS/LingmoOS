cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(calendar)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PLUGIN "calendar")

set(KF5_MINIMUM_VERSION "5.18.0")
set(QTXDG_MINIMUM_VERSION "3.3.1")

find_package(KF5WindowSystem ${KF5_MINIMUM_VERSION} REQUIRED)
find_package(Qt5Xdg ${QTXDG_MINIMUM_VERSION} REQUIRED)

find_package(Qt5 COMPONENTS Core Widgets Sql Svg Network  DBus LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Sql_INCLUDE_DIRS})
include_directories(/path/to/kdk)

pkg_check_modules(Gsetting REQUIRED gsettings-qt)
include_directories(${Gsetting_INCLUDE_DIRS})
link_directories(${Gsetting_PKG_LIBRARY_DIRS})
link_libraries(gsettings-qt.so)

pkg_check_modules(CALENDAR_DEPS REQUIRED glib-2.0)
link_libraries(glib-2.0.so)

#pkg_check_modules(CALENDAR_DEPS REQUIRED libqt5dbus5)
#link_libraries(libQt5DBus.so.5)

pkg_check_modules(LINGMOSDKSYSTIME_PKG lingmosdk-systime)
include_directories(${LINGMOSDKSYSTIME_PKG_INCLUDE_DIRS})
link_directories(${LINGMOSDKSYSTIME_PKG_LIBRARY_DIRS})
link_libraries(kydate.so)

pkg_check_modules(LINGMOSDKQTWIDGETS_PKG lingmosdk-qtwidgets)
include_directories(${LINGMOSDKQTWIDGETS_PKG_INCLUDE_DIRS})
link_directories(${LINGMOSDKQTWIDGETS_PKG_LIBRARY_DIRS})
link_libraries(lingmosdk-qtwidgets.so)

pkg_check_modules(LINGMOSDKKABASE_PKG lingmosdk-kabase)
include_directories(${LINGMOSDKKABASE_PKG_INCLUDE_DIRS})
link_directories(${LINGMOSDKKABASE_PKG_LIBRARY_DIRS})
link_libraries(lingmosdk-kabase.so)

pkg_check_modules(LINGMOSDKWAYLANDHELPER_PKG lingmosdk-waylandhelper)
include_directories(${LINGMOSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
link_directories(${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})
link_libraries(lingmosdk-waylandhelper.so)

message("${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS}")
include_directories(${CALENDAR_DEPS_INCLUDE_DIRS})

set(HEADERS
    lunarcalendarwidget/frmlunarcalendarwidget.h
    lunarcalendarwidget/lunarcalendarinfo.h
    lunarcalendarwidget/lunarcalendaritem.h
    lunarcalendarwidget/lunarcalendaryearitem.h
    lunarcalendarwidget/lunarcalendarmonthitem.h
    lunarcalendarwidget/lunarcalendarwidget.h
    lunarcalendarwidget/picturetowhite.h
    lunarcalendarwidget/customstylePushbutton.h
    lunarcalendarwidget/calendarcolor.h
    lunarcalendarwidget/schedulewidget.h
    lunarcalendarwidget/calendardatabase.h
    lunarcalendarwidget/mylabel.h
    lunarcalendarwidget/schedulemark.h
    lunarcalendarwidget/calendarinfo.h
    lunarcalendarwidget/customcalendar.h
#    lunarcalendarwidget/statelabel.h
    lunarcalendarwidget/schedulestruct.h
    lunarcalendarwidget/colorarea.h
    lunarcalendarwidget/schedule_item.h
    lunarcalendarwidget/morelabel.h
    lunarcalendarwidget/custommessagebox.h
    lunarcalendarwidget/datetimeedit.h
    lunarcalendarwidget/notificationsadaptor.h
    lunarcalendarwidget/customscrollarea.h
)
set(SOURCES
    lunarcalendarwidget/frmlunarcalendarwidget.cpp
    lunarcalendarwidget/lunarcalendarinfo.cpp
    lunarcalendarwidget/lunarcalendaritem.cpp
    lunarcalendarwidget/lunarcalendaryearitem.cpp
    lunarcalendarwidget/lunarcalendarmonthitem.cpp
    lunarcalendarwidget/lunarcalendarwidget.cpp
    lunarcalendarwidget/picturetowhite.cpp
    lunarcalendarwidget/customstylePushbutton.cpp
    lunarcalendarwidget/calendarcolor.cpp
    lunarcalendarwidget/schedulewidget.cpp
    lunarcalendarwidget/calendardatabase.cpp
    lunarcalendarwidget/mylabel.cpp
    lunarcalendarwidget/schedulemark.cpp
    lunarcalendarwidget/calendarinfo.cpp
    lunarcalendarwidget/customcalendar.cpp
    lunarcalendarwidget/colorarea.cpp
    lunarcalendarwidget/schedule_item.cpp
    lunarcalendarwidget/morelabel.cpp
    lunarcalendarwidget/custommessagebox.cpp
    lunarcalendarwidget/datetimeedit.cpp
    lunarcalendarwidget/main.cpp
    lunarcalendarwidget/notificationsadaptor.cpp
    lunarcalendarwidget/customscrollarea.cpp
)

set(UIS
    calendar/lingmowebviewdialog.ui
    lunarcalendarwidget/frmlunarcalendarwidget.ui
)

add_executable(lingmo-calendar  ${SOURCES} ${HEADERS} ${UIS})

target_link_libraries(lingmo-calendar
    Qt5::Widgets
    Qt5::Svg
    KF5::WindowSystem
    ${Gsetting_LIBRARIES}
    Qt5::Sql
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    pthread
)

file(GLOB_RECURSE DESKTOP_FILE "resources/calendar.desktop")
set(PACKAGE_DATA_DIR "/usr/share/calendar")

file(GLOB_RECURSE GSETTING_SCHEMAS "resources/org.lingmo.calendar.plugin.gschema.xml")
set(SCHEMAS_INSTALL_DIR "/usr/share/glib-2.0/schemas/")

#install(TARGETS lingmo-calendar DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-panel/")
install(TARGETS lingmo-calendar DESTINATION /usr/bin)
install(FILES ${DESKTOP_FILE} DESTINATION "/etc/xdg/autostart")
install(FILES ${GSETTING_SCHEMAS} DESTINATION ${SCHEMAS_INSTALL_DIR})
install(DIRECTORY html/ DESTINATION ${PACKAGE_DATA_DIR}/plugin-calendar/html)

file(GLOB TS_FILES "translation/*.ts")

# 加载翻译文件
 file(GLOB TS_FILES "${PROJECT_SOURCE_DIR}/translation/*.ts" )
# 更新翻译文件并创建.qm文件
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})
set(${PLUGIN}_QM_FILES ${QM_FILES})

# #安装翻译文件
set(CALENDAR_TRANSLATION_DIR "${PACKAGE_DATA_DIR}/plugin-calendar/translation")
add_compile_definitions(CALENDAR_TRANSLATION_DIR="${CALENDAR_TRANSLATION_DIR}")

install(FILES ${QM_FILES} DESTINATION ${CALENDAR_TRANSLATION_DIR})
