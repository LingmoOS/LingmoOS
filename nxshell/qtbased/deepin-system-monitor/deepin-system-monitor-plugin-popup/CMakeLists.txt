cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(deepin-system-monitor-plugin-popup LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(APP_RES_ROOT asserts)
set(APP_RESOURCES ${APP_RES_ROOT}/${PROJECT_NAME}.qrc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

set(MAIN_APP_DIR ${CMAKE_SOURCE_DIR}/deepin-system-monitor-main)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions("-DQT_NO_DEBUG_OUTPUT")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/environments.h.in ${CMAKE_CURRENT_BINARY_DIR}/environments.h @ONLY)

include(CheckIncludeFiles)
check_include_files("sys/random.h" HAVE_SYS_RANDOM_H)
include(CheckSymbolExists)
check_symbol_exists(syscall "unistd.h" HAVE_SYS_CALL)
check_symbol_exists(SYS_getrandom "sys/syscall.h" HAVE_SYS_CALL)
include(FindPkgConfig)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)

find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5Svg REQUIRED)
find_library(LIB_XCB NAMES xcb REQUIRED)
find_library(LIB_XEXT NAMES Xext REQUIRED)
find_library(LIB_ICCCM NAMES xcb-icccm REQUIRED)
find_package(ICU COMPONENTS i18n uc REQUIRED)

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-Map=deepin-system-monitor-pluign-popup.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-Map=deepin-system-monitor-popup.map")
#pkg_check_modules(QGSettings REQUIRED gsettings-qt)

#pkg_check_modules(DdeDockInterface REQUIRED dde-dock)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")

pkg_search_module(LIB_NL3 REQUIRED libnl-3.0)
pkg_search_module(LIB_NL3_ROUTE REQUIRED libnl-route-3.0)
pkg_search_module(LIB_UDEV REQUIRED libudev)
include_directories(${LIB_NL3_INCLUDE_DIRS})
include_directories(${LIB_NL3_ROUTE_INCLUDE_DIRS})
include_directories(${LIB_UDEV_INCLUDE_DIRS})

#include_directories(${QGSettings_INCLUDE_DIRS})

include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})
include_directories(${MAIN_APP_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

FILE(GLOB_RECURSE SRCS "*.h" "*.cpp")

SET(HPP_MODEL
    ${MAIN_APP_DIR}/model/cpu_info_model.h
    ${MAIN_APP_DIR}/model/cpu_stat_model.h
    ${MAIN_APP_DIR}/model/cpu_list_model.h
    model/process_sort_filter_proxy_model.h
    model/process_table_model.h
)

SET(CPP_MODEL
    ${MAIN_APP_DIR}/model/cpu_info_model.cpp
    ${MAIN_APP_DIR}/model/cpu_stat_model.cpp
    ${MAIN_APP_DIR}/model/cpu_list_model.cpp
    model/process_sort_filter_proxy_model.cpp
    model/process_table_model.cpp
)

SET(HPP_GLOBAL
    common/datacommon.h
    ${MAIN_APP_DIR}/common/common.h
    common/utils.h
    ${MAIN_APP_DIR}/common/hash.h
    ${MAIN_APP_DIR}/common/sample.h
    ${MAIN_APP_DIR}/stack_trace.h
    ${MAIN_APP_DIR}/common/thread_manager.h
    ${MAIN_APP_DIR}/common/time_period.h
    ${MAIN_APP_DIR}/common/base_thread.h
    ${MAIN_APP_DIR}/common/error_context.h
    ${MAIN_APP_DIR}/common/han_latin.h
    ${MAIN_APP_DIR}/settings.h
    ${MAIN_APP_DIR}/common/perf.h
)

SET(CPP_GLOBAL
    common/utils.cpp
    ${MAIN_APP_DIR}/common/hash.cpp
    ${MAIN_APP_DIR}/common/common.cpp
    ${MAIN_APP_DIR}/common/thread_manager.cpp
    ${MAIN_APP_DIR}/common/time_period.cpp
    ${MAIN_APP_DIR}/common/error_context.cpp
    ${MAIN_APP_DIR}/common/han_latin.cpp
    ${MAIN_APP_DIR}/settings.cpp
    ${MAIN_APP_DIR}/common/perf.cpp
)

SET(HPP_SYSTEM
    ${MAIN_APP_DIR}/system/private/cpu_p.h
    ${MAIN_APP_DIR}/system/private/cpu_set_p.h
    ${MAIN_APP_DIR}/system/private/mem_p.h
    ${MAIN_APP_DIR}/system/private/sys_info_p.h
    ${MAIN_APP_DIR}/system/private/block_device_p.h
    ${MAIN_APP_DIR}/system/diskio_info.h
    system/cpu_set.h
    ${MAIN_APP_DIR}/system/cpu.h
    system/device_db.h
    ${MAIN_APP_DIR}/system/mem.h
    ${MAIN_APP_DIR}/system/net_info.h
    ${MAIN_APP_DIR}/system/packet.h
    ${MAIN_APP_DIR}/system/sys_info.h

    ${MAIN_APP_DIR}/system/system_monitor_thread.h
    ${MAIN_APP_DIR}/system/system_monitor.h
    ${MAIN_APP_DIR}/system/block_device_info_db.h
    ${MAIN_APP_DIR}/system/block_device.h
)

SET(CPP_SYSTEM
    ${MAIN_APP_DIR}/system/diskio_info.cpp
    system/cpu_set.cpp
    ${MAIN_APP_DIR}/system/cpu.cpp
    system/device_db.cpp
    ${MAIN_APP_DIR}/system/mem.cpp
    ${MAIN_APP_DIR}/system/net_info.cpp
    ${MAIN_APP_DIR}/system/sys_info.cpp
    ${MAIN_APP_DIR}/system/system_monitor_thread.cpp
    ${MAIN_APP_DIR}/system/system_monitor.cpp
    ${MAIN_APP_DIR}/system/block_device_info_db.cpp
    ${MAIN_APP_DIR}/system/block_device.cpp
)

SET(HPP_GUI
    gui/base/base_table_view.h
    ${MAIN_APP_DIR}/gui/base/base_header_view.h
    gui/base/base_item_delegate.h
    gui/mainwindow.h
    gui/datadealsingleton.h
    gui/process_table_view.h
    ${MAIN_APP_DIR}/gui/ui_common.h
    gui/cpu_widget.h
    gui/disk_widget.h
    gui/memory_widget.h
    gui/net_widget.h
    gui/process_widget.h
)


SET(CPP_GUI
    gui/base/base_table_view.cpp
    ${MAIN_APP_DIR}/gui/base/base_header_view.cpp
    gui/base/base_item_delegate.cpp
    gui/mainwindow.cpp
    gui/datadealsingleton.cpp
    gui/process_table_view.cpp
    ${MAIN_APP_DIR}/gui/ui_common.cpp
    gui/cpu_widget.cpp
    gui/disk_widget.cpp
    gui/memory_widget.cpp
    gui/net_widget.cpp
    gui/process_widget.cpp
)


SET(HPP_WM
    ${MAIN_APP_DIR}/wm/wm_window_list.h
    ${MAIN_APP_DIR}/wm/wm_atom.h
    ${MAIN_APP_DIR}/wm/wm_connection.h
    ${MAIN_APP_DIR}/wm/wm_info.h
    ${MAIN_APP_DIR}/wm/wm_window_tree.h
)

SET(CPP_WM
    ${MAIN_APP_DIR}/wm/wm_window_list.cpp
    ${MAIN_APP_DIR}/wm/wm_atom.cpp
    ${MAIN_APP_DIR}/wm/wm_connection.cpp
    ${MAIN_APP_DIR}/wm/wm_info.cpp
    ${MAIN_APP_DIR}/wm/wm_window_tree.cpp
)

SET(HPP_DBUS
    dbus/systemmonitordbusadaptor.h
    dbus/dbusdockinterface.h
    dbus/dbuscallmaininterface.h
    dbus/dbusayatanainterface.h
)

SET(CPP_DBUS
    dbus/systemmonitordbusadaptor.cpp
    dbus/dbusdockinterface.cpp
    dbus/dbuscallmaininterface.cpp
    dbus/dbusayatanainterface.cpp
)

SET(HPP_PROCESS
    ${MAIN_APP_DIR}/process/private/process_p.h
    ${MAIN_APP_DIR}/process/process_icon_cache.h
    ${MAIN_APP_DIR}/process/process_set.h
    process/process.h
    process/process_db.h
    ${MAIN_APP_DIR}/process/process_icon.h
    ${MAIN_APP_DIR}/process/desktop_entry_cache.h
    ${MAIN_APP_DIR}/process/desktop_entry_cache_updater.h
    ${MAIN_APP_DIR}/process/process_name.h
    ${MAIN_APP_DIR}/process/process_name_cache.h
    ${MAIN_APP_DIR}/process/process_controller.h
)

SET(CPP_PROCESS
    ${MAIN_APP_DIR}/process/process_icon_cache.cpp
    ${MAIN_APP_DIR}/process/process_set.cpp
    process/process.cpp
    process/process_db.cpp
    ${MAIN_APP_DIR}/process/process_icon.cpp
    ${MAIN_APP_DIR}/process/desktop_entry_cache.cpp
    ${MAIN_APP_DIR}/process/desktop_entry_cache_updater.cpp
    ${MAIN_APP_DIR}/process/process_name.cpp
    ${MAIN_APP_DIR}/process/process_name_cache.cpp
    ${MAIN_APP_DIR}/process/process_controller.cpp
)
set(APP_HPP
    ${CMAKE_HOME_DIRECTORY}/config.h
    application.h
    ddlog.h
    logger.h
    ${HPP_MODEL}
    ${HPP_SYSTEM}
    ${HPP_GLOBAL}
    ${HPP_GUI}
    ${HPP_DBUS}
    ${HPP_PROCESS}
    ${HPP_WM}
)

set(APP_CPP
    application.cpp
    main.cpp
    logger.cpp
    ${CPP_MODEL}
    ${CPP_SYSTEM}
    ${CPP_GLOBAL}
    ${CPP_GUI}
    ${CPP_DBUS}
    ${CPP_PROCESS}
    ${CPP_WM}
)

add_executable(${PROJECT_NAME}
    ${APP_HPP}
    ${APP_CPP}
    ${PROJECT_SOURCE_DIR}/${APP_RESOURCES}    
)

target_include_directories(${PROJECT_NAME} PUBLIC
#    ${Qt5Widgets_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${DtkGui_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
    Dtk::Core
)
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::Svg
    dtkcore
    dtkwidget
    dtkgui
    ICU::i18n
    ICU::uc
    ${LIB_XCB}
    ${LIB_XEXT}
    ${LIB_ICCCM}
    ${LIB_NL3_LIBRARIES}
    ${LIB_NL3_ROUTE_LIBRAIES}
    ${LIB_UDEV_LIBRARIES}
    ${LIB_DDEDOCK}
#    gsettings-qt
    Dtk::Core
)


#生成qm
file(GLOB APP_TS_FILES LIST_DIRECTORIES false translations/*.ts)
qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})
add_custom_target(POPUP_QMFILES ALL DEPENDS ${APP_QM_FILES})

#
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.SystemMonitorPluginPopup.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-system-monitor-plugin-popup.desktop DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart)
#安装翻译文件
install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-system-monitor-plugin-popup/translations)
