# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(PROJECT_NAME deepin-system-monitor)
set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

ADD_COMPILE_OPTIONS(-fno-access-control)
add_definitions(-DENABLE_AUTO_UNIT_TEST)

set(APP_RES_ROOT assets)
set(APP_RESOURCES ../deepin-system-monitor-main/${APP_RES_ROOT}/${PROJECT_NAME}.qrc)
set(DESKTOP_FILE ${PROJECT_NAME}.desktop)
set(POLICY_FILE com.deepin.pkexec.${PROJECT_NAME}.policy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-Map=deepin-system-monitor-test.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-Map=deepin-system-monitor-test.map")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions("-DQT_NO_DEBUG_OUTPUT")
endif()

configure_file(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/environments.h.in ${CMAKE_CURRENT_BINARY_DIR}/environments.h @ONLY)

include(CheckIncludeFiles)
check_include_files ("sys/random.h" HAVE_SYS_RANDOM_H)
include(CheckSymbolExists)
check_symbol_exists(syscall "unistd.h" HAVE_SYS_CALL)
check_symbol_exists(SYS_getrandom "sys/syscall.h" HAVE_SYS_GETRANDOM)
configure_file(${CMAKE_HOME_DIRECTORY}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include(FindPkgConfig)

find_package(KF5Wayland)
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
find_library(LIB_PCAP NAMES pcap REQUIRED)
find_library(LIB_XCB NAMES xcb REQUIRED)
find_library(LIB_XEXT NAMES Xext REQUIRED)
find_library(LIB_ICCCM NAMES xcb-icccm REQUIRED)
find_package(ICU COMPONENTS i18n uc REQUIRED)

# For UT Test
find_package(GTest REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

pkg_search_module(LIB_NL3 REQUIRED libnl-3.0)
pkg_search_module(LIB_NL3_ROUTE REQUIRED libnl-route-3.0)
pkg_search_module(LIB_UDEV REQUIRED libudev)
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})
include_directories(${LIB_NL3_INCLUDE_DIRS})
include_directories(${LIB_NL3_ROUTE_INCLUDE_DIRS})
include_directories(${LIB_UDEV_INCLUDE_DIRS})
include_directories(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main)
include_directories(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui)
include_directories(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty)
include_directories(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include)
include_directories(${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src)



set(HPP_GLOBAL
    ${CMAKE_HOME_DIRECTORY}/config.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/environments.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/accessible.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/accessibledefine.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/application.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/stack_trace.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/cpu_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/memory_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/network_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_cpu_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_memory_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_network_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_disk_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/smooth_curve_generator.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/settings.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/detailwidgetmanager.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/systemprotectionsetting.h
)
set(CPP_GLOBAL
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/application.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/cpu_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/memory_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/network_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_cpu_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_memory_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_network_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/compact_disk_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/smooth_curve_generator.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/settings.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/detailwidgetmanager.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/systemprotectionsetting.cpp
)

set(HPP_COMMON
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/common.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/error_context.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/hash.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/han_latin.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/perf.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/base_thread.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/thread_manager.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/time_period.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/sample.h
)
set(CPP_COMMON
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/common.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/error_context.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/hash.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/han_latin.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/perf.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/thread_manager.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/common/time_period.cpp
)

set(HPP_DBUS
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_manager_interface.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_service_interface.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_unit_interface.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbus_properties_interface.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbus_common.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/environment_file.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/unit_file_info.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/unit_info.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbusforsystemomonitorpluginservce.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbus_object.h
)
set(CPP_DBUS
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/environment_file.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/unit_file_info.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/unit_info.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_manager_interface.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_service_interface.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/systemd1_unit_interface.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbus_properties_interface.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbusforsystemomonitorpluginservce.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/dbus/dbus_object.cpp
)

set(HPP_MODEL
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/process_table_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/process_sort_filter_proxy_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/system_service_table_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/system_service_sort_filter_proxy_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_info_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_stat_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_list_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_list_sort_filter_proxy_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_info_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_stat_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_addr_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_info_sort_filter_proxy_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_stat_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_info_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_info_sort_filter_proxy_model.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/model_manager.h
)
set(CPP_MODEL
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/system_service_table_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/system_service_sort_filter_proxy_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/process_table_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/process_sort_filter_proxy_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_info_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_stat_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_list_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/cpu_list_sort_filter_proxy_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_info_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_stat_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_addr_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/netif_info_sort_filter_proxy_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_info_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_stat_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/block_dev_info_sort_filter_proxy_model.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/model/model_manager.cpp
)

set(HPP_GUI
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_detail_item_delegate.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_table_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_item_delegate.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_header_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_detail_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_commandlink_button.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/ui_common.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/toolbar.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/main_window.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_table_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_page_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/service_name_sub_input_dialog.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/system_service_table_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/system_service_page_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/monitor_expand_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/monitor_compact_view.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/priority_slider.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/kill_process_confirm_dialog.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_attribute_dialog.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/error_dialog.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/xwin_kill_preview_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/xwin_kill_preview_background_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_detail_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_summary_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_stat_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_detail_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_summary_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_detail_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_stat_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_item_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_summary_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/detail_view_stacked_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/chart_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_stat_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/animation_stackedwidget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/cpu_detail_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/cpu_summary_view_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_item_widget.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/custombuttonbox.h
)
set(CPP_GUI
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_detail_item_delegate.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_table_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_item_delegate.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_header_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_detail_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/base/base_commandlink_button.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/toolbar.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/system_service_table_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/main_window.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/system_service_page_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_page_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/service_name_sub_input_dialog.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_table_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/error_dialog.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/monitor_expand_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/monitor_compact_view.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/kill_process_confirm_dialog.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/process_attribute_dialog.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/priority_slider.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/ui_common.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/xwin_kill_preview_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/xwin_kill_preview_background_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_detail_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_summary_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/mem_stat_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_detail_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_summary_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_detail_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_summary_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_stat_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/netif_item_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/detail_view_stacked_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/chart_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/animation_stackedwidget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/cpu_detail_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/cpu_summary_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_item_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/block_dev_stat_view_widget.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui/dialog/custombuttonbox.cpp
)

set(HPP_PROCESS
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/private/process_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_set.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_icon.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_icon_cache.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_name.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_name_cache.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/priority_controller.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_controller.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/desktop_entry_cache.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/desktop_entry_cache_updater.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_db.h
)
set(CPP_PROCESS
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_set.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_icon.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_icon_cache.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_name.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_name_cache.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/priority_controller.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_controller.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/desktop_entry_cache.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/desktop_entry_cache_updater.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/process/process_db.cpp
)

set(HPP_SERVICE
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/service_manager_worker.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/service_manager.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/system_service_entry_data.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/system_service_entry.h
)
set(CPP_SERVICE
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/service_manager_worker.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/service_manager.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/system_service_entry_data.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/service/system_service_entry.cpp
)

set(HPP_SYSTEM
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/mem_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/cpu_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/cpu_set_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/block_device_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/netif_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/private/sys_info_p.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/system_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/system_monitor_thread.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/device_id_cache.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/packet.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_info_db.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_monitor.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_monitor_thread.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_packet_capture.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_packet_parser.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/mem.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/cpu.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/cpu_set.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/block_device.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/block_device_info_db.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/device_db.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/sys_info.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/udev.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/udev_device.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netlink.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_addr.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_hwaddr.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_link.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/wireless.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/diskio_info.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/net_info.h
)

set(CPP_SYSTEM
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/system_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/system_monitor_thread.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/device_id_cache.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_monitor.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_monitor_thread.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_packet_capture.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_packet_parser.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/device_db.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netif_info_db.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/mem.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/cpu.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/cpu_set.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/block_device.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/block_device_info_db.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/sys_info.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/udev.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/udev_device.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/netlink.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_addr.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_hwaddr.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/nl_link.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/wireless.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/diskio_info.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/system/net_info.cpp
)

set(HPP_WM
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_atom.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_connection.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_info.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_window_list.h
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_window_tree.h
)

set(CPP_WM
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_atom.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_connection.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_info.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_window_list.cpp
    ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/wm/wm_window_tree.cpp
)

set(LSCPU
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/calculate.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/cell.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/column.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/grouping.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/init.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/iter.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/line.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/print-api.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/print.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/symbols.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/table.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/version.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/walk.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-arm.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-cpu.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-cputype.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-dmi.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-topology.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu-virt.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/cpuset.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/path.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/strutils.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/buffer.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/mbsalign.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/color-names.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/jsonwrt.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/ttyutils.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/c_strtod.c
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/fileutils.c
)

set(LSCPU_INCLUDE
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/libsmartcols.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src/smartcolsP.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/all-io.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/encode.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/minix.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pty-session.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/bitops.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/env.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/monotonic.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pwdutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/blkdev.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/exec_shell.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/namespace.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/randutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/buffer.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/exitcodes.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/nls.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/rpmatch.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/canonicalize.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/fileutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/optutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/selinux-utils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/caputils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/fuzz.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pager.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/sha1.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/carefulputc.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/idcache.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/partx.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/signames.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/cctype.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/ismounted.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/path.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/statfs_magic.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/c.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/iso9660.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pathnames.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/strutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/closestream.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/jsonwrt.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pidfd-utils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/strv.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/color-names.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/linux_version.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/plymouth-ctrl.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/swapheader.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/colors.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/list.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/procutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/swapprober.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/cpuset.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/loopdev.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-bsd.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/sysfs.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/crc32c.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/mangle.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-gpt-partnames.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/timer.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/crc32.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/match.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-mbr.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/timeutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/c_strtod.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/mbsalign.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-mbr-partnames.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/ttyutils.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/debug.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/mbsedit.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-sgi.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/widechar.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/debugobj.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/md5.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/pt-sun.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include/xalloc.h
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/lscpu.h
)


set(DMIDECODE_HEADS
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmidecode.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmioem.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmiopt.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmioutput.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/types.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/util.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/version.h
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/vpdopt.h
       )
set(DMIDECODE
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmidecode.c
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmioem.c
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmiopt.c
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/dmioutput.c
            ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/dmidecode/util.c
       )
set(APP_HPP
    ${HPP_GLOBAL}
    ${HPP_COMMON}
    ${HPP_DBUS}
    ${HPP_MODEL}
    ${HPP_GUI}
    ${HPP_PROCESS}
    ${HPP_SERVICE}
    ${HPP_SYSTEM}
    ${HPP_WM}
    ${LSCPU_INCLUDE}
    ${DMIDECODE_HEADS}
)

set(APP_CPP
    ${CPP_GLOBAL}
    ${CPP_COMMON}
    ${CPP_DBUS}
    ${CPP_MODEL}
    ${CPP_GUI}
    ${CPP_PROCESS}
    ${CPP_SERVICE}
    ${CPP_SYSTEM}
    ${CPP_WM}
    ${LSCPU}
    ${DMIDECODE}
)

file(GLOB APP_TS_FILES LIST_DIRECTORIES false ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/translations/*.ts)
qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})
add_custom_target(QMFILES_TEST ALL DEPENDS ${APP_QM_FILES})

find_program(POLICY_TS_CONV deepin-policy-ts-convert REQUIRED)
add_custom_target(APP_POLICY_FILE_TEST
    ALL
    COMMAND ${POLICY_TS_CONV} ts2policy ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/translations/policy/${POLICY_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/translations/policy/ ${POLICY_FILE}.tmp
    COMMAND mv ${POLICY_FILE}.tmp ${POLICY_FILE}
)

find_program(DESKTOP_TS_CONV deepin-desktop-ts-convert REQUIRED)
add_custom_target(APP_DESKTOP_FILE_TEST
    ALL
    COMMAND ${DESKTOP_TS_CONV} ts2desktop ${CMAKE_CURRENT_SOURCE_DIR}/translations/desktop/${DESKTOP_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/translations/desktop/ ${DESKTOP_FILE}.tmp
    COMMAND mv ${DESKTOP_FILE}.tmp ${DESKTOP_FILE}
)

file(GLOB_RECURSE UT_CPP ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE UT_HPP ${CMAKE_CURRENT_LIST_DIR}/*.h)

add_executable(${PROJECT_NAME_TEST}
    ${UT_CPP}
    ${APP_HPP}
    ${APP_CPP}
    ${APP_RESOURCES}
)

set_target_properties(${PROJECT_NAME_TEST}
        PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

## target_compile_definitions(${PROJECT_NAME_TEST}
##         PRIVATE
##         ${WEBRTC_DEFINITIONS}
## )

target_include_directories(${PROJECT_NAME_TEST}
        PRIVATE
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5DBus_LIBRARIES}
        ${GTEST_INCLUDE_DIRS}
        ${LIB_NL3_INCLUDE_DIRS}
        ${LIB_NL3_ROUTE_INCLUDE_DIRS}
        ${LIB_UDEV_INCLUDE_DIRS}
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/gui
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/include
        ${CMAKE_HOME_DIRECTORY}/${PROJECT_NAME}-main/3rdparty/libsmartcols/src
        )

target_link_libraries(${PROJECT_NAME_TEST}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
    Qt5::X11Extras
    dtkcore
    dtkwidget
    dtkgui
    KF5::WaylandClient
    KF5::WaylandServer
    ${LIB_PCAP}
    ICU::i18n
    ICU::uc
    ${LIB_XCB}
    ${LIB_XEXT}
    ${LIB_ICCCM}
    ${LIB_NL3_LIBRARIES}
    ${LIB_NL3_ROUTE_LIBRARIES}
    ${LIB_UDEV_LIBRARIES}
    Threads::Threads
    Qt5::Test
    ${GTEST_LIBRARYS}
    ${GTEST_MAIN_LIBRARYS}
    gmock
    gmock_main
    gtest
    gtest_main
)

# INSTALL(TARGETS ${PROJECT_NAME_TEST} DESTINATION bin)

#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
#    COMMAND mkdir -p tests/coverageResult
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

    #1.清理原先的gcov信息
#    COMMAND lcov --directory ./tests/CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
#    COMMAND lcov --directory ./tests/CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}


    #2.收集gcov信息到.info文件中
#    COMMAND lcov --directory . --capture --output-file ./tests/coverageResult/${PROJECT_NAME}_Coverage.info

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
#    COMMAND echo " =================== do filter begin ==================== "
#    COMMAND lcov --remove ./tests/coverageResult/${PROJECT_NAME}_Coverage.info
#    '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/tests/*' '*/googletest/*' '*/usr/include/*' '*/src/bak/*' '*/src/core/*' '*/src/math/*'
#    -o ./tests/coverageResult/${PROJECT_NAME}_Coverage.info
#    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
#    COMMAND genhtml -o ./tests/coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

#    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "

#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")
if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")
if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()
