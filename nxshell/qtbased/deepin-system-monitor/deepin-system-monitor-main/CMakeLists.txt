cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(deepin-system-monitor)

option(USE_DEEPIN_WAYLAND "option for wayland support" ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(APP_RES_ROOT assets)
set(APP_RESOURCES ${APP_RES_ROOT}/${PROJECT_NAME}.qrc)
set(DESKTOP_FILE ${PROJECT_NAME}.desktop)
set(POLICY_FILE com.deepin.pkexec.${PROJECT_NAME}.policy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions("-DQT_NO_DEBUG_OUTPUT")
endif()
if (USE_DEEPIN_WAYLAND)
  add_definitions("-DUSE_DEEPIN_WAYLAND")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/environments.h.in ${CMAKE_CURRENT_BINARY_DIR}/environments.h @ONLY)

include(CheckIncludeFiles)
check_include_files ("sys/random.h" HAVE_SYS_RANDOM_H)
include(CheckSymbolExists)
check_symbol_exists(syscall "unistd.h" HAVE_SYS_CALL)
check_symbol_exists(SYS_getrandom "sys/syscall.h" HAVE_SYS_GETRANDOM)
include(FindPkgConfig)
if (USE_DEEPIN_WAYLAND)
    find_package(DWayland)
    find_package(KF5Wayland)
    if (DWayland_FOUND)
        message("DWayland founded.")
        add_definitions(-DDWAYLAND)
    elseif(KF5Wayland_FOUND)
        message("KF5Wayland founded.")
        add_definitions(-DKF5WAYLAND)
    endif()
endif()

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

#pkg_search_module(DFrameworkDBus REQUIRED dframeworkdbus)  # chinalife
pkg_search_module(LIB_NL3 REQUIRED libnl-3.0)
pkg_search_module(LIB_NL3_ROUTE REQUIRED libnl-route-3.0)
pkg_search_module(LIB_UDEV REQUIRED libudev)
include_directories(${DtkCore_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkWidget_INCLUDE_DIRS})

include_directories(${LIB_NL3_INCLUDE_DIRS})
include_directories(${LIB_NL3_ROUTE_INCLUDE_DIRS})
include_directories(${LIB_UDEV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libsmartcols/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
#include_directories(${DFrameworkDBus_INCLUDE_DIRS})  # chinalife

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-Map=deepin-system-monitor-main.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-Map=deepin-system-monitor-main.map")

set(HPP_GLOBAL
    environments.h
    accessible.h
    accessibledefine.h
    application.h
    stack_trace.h
    cpu_monitor.h
    memory_monitor.h
    network_monitor.h
    compact_cpu_monitor.h
    compact_memory_monitor.h
    compact_network_monitor.h
    compact_disk_monitor.h
    smooth_curve_generator.h
    settings.h
    detailwidgetmanager.h
    logger.h
    ddlog.h
)
set(CPP_GLOBAL
    main.cpp
    application.cpp
    cpu_monitor.cpp
    memory_monitor.cpp
    network_monitor.cpp
    compact_cpu_monitor.cpp
    compact_memory_monitor.cpp
    compact_network_monitor.cpp
    compact_disk_monitor.cpp
    smooth_curve_generator.cpp
    settings.cpp
    detailwidgetmanager.cpp
    logger.cpp
)

set(HPP_COMMON
    common/common.h
    common/error_context.h
    common/hash.h
    common/han_latin.h
    common/perf.h
    common/base_thread.h
    common/thread_manager.h
    common/time_period.h
    common/sample.h
    common/eventlogutils.h
)
set(CPP_COMMON
    common/common.cpp
    common/error_context.cpp
    common/hash.cpp
    common/han_latin.cpp
    common/perf.cpp
    common/thread_manager.cpp
    common/time_period.cpp
    common/eventlogutils.cpp
)

set(HPP_DBUS
    dbus/systemd1_manager_interface.h
    dbus/systemd1_service_interface.h
    dbus/systemd1_unit_interface.h
    dbus/dbus_properties_interface.h
    dbus/dbus_common.h
    dbus/environment_file.h
    dbus/unit_file_info.h
    dbus/unit_info.h
    dbus/dbusforsystemomonitorpluginservce.h
    dbus/dbus_object.h
    dbus/dbusalarmnotify.h
)
set(CPP_DBUS
    dbus/environment_file.cpp
    dbus/unit_file_info.cpp
    dbus/unit_info.cpp
    dbus/systemd1_manager_interface.cpp
    dbus/systemd1_service_interface.cpp
    dbus/systemd1_unit_interface.cpp
    dbus/dbus_properties_interface.cpp
    dbus/dbusforsystemomonitorpluginservce.cpp
    dbus/dbus_object.cpp
    dbus/dbusalarmnotify.cpp
)

set(HPP_MODEL
    model/process_table_model.h
    model/process_sort_filter_proxy_model.h
    model/system_service_table_model.h
    model/system_service_sort_filter_proxy_model.h
    model/cpu_info_model.h
    model/cpu_stat_model.h
    model/cpu_list_model.h
    model/cpu_list_sort_filter_proxy_model.h
    model/netif_info_model.h
    model/netif_stat_model.h
    model/netif_addr_model.h
    model/netif_info_sort_filter_proxy_model.h
    model/block_dev_stat_model.h
    model/block_dev_info_model.h
    model/block_dev_info_sort_filter_proxy_model.h
    model/model_manager.h
    model/accounts_info_model.h
    model/user.h

)
set(CPP_MODEL
    model/system_service_table_model.cpp
    model/system_service_sort_filter_proxy_model.cpp
    model/process_table_model.cpp
    model/process_sort_filter_proxy_model.cpp
    model/cpu_info_model.cpp
    model/cpu_stat_model.cpp
    model/cpu_list_model.cpp
    model/cpu_list_sort_filter_proxy_model.cpp
    model/netif_info_model.cpp
    model/netif_stat_model.cpp
    model/netif_addr_model.cpp
    model/netif_info_sort_filter_proxy_model.cpp
    model/block_dev_info_model.cpp
    model/block_dev_stat_model.cpp
    model/block_dev_info_sort_filter_proxy_model.cpp
    model/model_manager.cpp
    model/accounts_info_model.cpp
    model/user.cpp
)

set(HPP_GUI
    gui/base/base_detail_item_delegate.h
    gui/base/base_table_view.h
    gui/base/base_item_delegate.h
    gui/base/base_header_view.h
    gui/base/base_detail_view_widget.h
    gui/base/base_commandlink_button.h
    gui/ui_common.h
    gui/toolbar.h
    gui/main_window.h
    gui/process_table_view.h
    gui/process_page_widget.h
    gui/service_name_sub_input_dialog.h
    gui/system_service_table_view.h
    gui/system_service_page_widget.h
    gui/monitor_expand_view.h
    gui/monitor_compact_view.h
    gui/priority_slider.h
    gui/kill_process_confirm_dialog.h
    gui/process_attribute_dialog.h
    gui/dialog/error_dialog.h
    gui/xwin_kill_preview_widget.h
    gui/xwin_kill_preview_background_widget.h
    gui/mem_detail_view_widget.h
    gui/mem_summary_view_widget.h
    gui/mem_stat_view_widget.h
    gui/block_dev_detail_view_widget.h
    gui/block_dev_summary_view_widget.h
    gui/netif_detail_view_widget.h
    gui/netif_stat_view_widget.h
    gui/netif_item_view_widget.h
    gui/netif_summary_view_widget.h
    gui/detail_view_stacked_widget.h
    gui/chart_view_widget.h
    gui/block_dev_stat_view_widget.h
    gui/animation_stackedwidget.h
    gui/cpu_detail_widget.h
    gui/cpu_summary_view_widget.h
    gui/block_dev_item_widget.h
    gui/dialog/systemprotectionsetting.h
    gui/dialog/custombuttonbox.h
    gui/user_page_widget.h
    gui/accounts_widget.h
    gui/onlineicon.h
    gui/user_list_view.h
)
set(CPP_GUI
    gui/base/base_detail_item_delegate.cpp
    gui/base/base_table_view.cpp
    gui/base/base_item_delegate.cpp
    gui/base/base_header_view.cpp
    gui/base/base_detail_view_widget.cpp
    gui/base/base_commandlink_button.cpp
    gui/toolbar.cpp
    gui/system_service_table_view.cpp
    gui/main_window.cpp
    gui/system_service_page_widget.cpp
    gui/process_page_widget.cpp
    gui/service_name_sub_input_dialog.cpp
    gui/process_table_view.cpp
    gui/dialog/error_dialog.cpp
    gui/monitor_expand_view.cpp
    gui/monitor_compact_view.cpp
    gui/kill_process_confirm_dialog.cpp
    gui/process_attribute_dialog.cpp
    gui/priority_slider.cpp
    gui/ui_common.cpp
    gui/xwin_kill_preview_widget.cpp
    gui/xwin_kill_preview_background_widget.cpp
    gui/mem_detail_view_widget.cpp
    gui/mem_summary_view_widget.cpp
    gui/mem_stat_view_widget.cpp
    gui/block_dev_detail_view_widget.cpp
    gui/block_dev_summary_view_widget.cpp
    gui/netif_detail_view_widget.cpp
    gui/netif_summary_view_widget.cpp
    gui/netif_stat_view_widget.cpp
    gui/netif_item_view_widget.cpp
    gui/detail_view_stacked_widget.cpp
    gui/chart_view_widget.cpp
    gui/animation_stackedwidget.cpp
    gui/cpu_detail_widget.cpp
    gui/cpu_summary_view_widget.cpp
    gui/block_dev_item_widget.cpp
    gui/block_dev_stat_view_widget.cpp
    gui/dialog/systemprotectionsetting.cpp
    gui/dialog/settingsdialog.cpp
    gui/dialog/custombuttonbox.cpp
    gui/user_page_widget.cpp
    gui/accounts_widget.cpp
    gui/onlineicon.cpp
    gui/user_list_view.cpp
)

set(HPP_PROCESS
    process/private/process_p.h
    process/process.h
    process/process_set.h
    process/process_icon.h
    process/process_icon_cache.h
    process/process_name.h
    process/process_name_cache.h
    process/priority_controller.h
    process/process_controller.h
    process/desktop_entry_cache.h
    process/desktop_entry_cache_updater.h
    process/process_db.h
)
set(CPP_PROCESS
    process/process.cpp
    process/process_set.cpp
    process/process_icon.cpp
    process/process_icon_cache.cpp
    process/process_name.cpp
    process/process_name_cache.cpp
    process/priority_controller.cpp
    process/process_controller.cpp
    process/desktop_entry_cache.cpp
    process/desktop_entry_cache_updater.cpp
    process/process_db.cpp
)

set(HPP_SERVICE
    service/service_manager_worker.h
    service/service_manager.h
    service/system_service_entry_data.h
    service/system_service_entry.h
)
set(CPP_SERVICE
    service/service_manager_worker.cpp
    service/service_manager.cpp
    service/system_service_entry_data.cpp
    service/system_service_entry.cpp
)

set(HPP_SYSTEM
    system/private/mem_p.h
    system/private/cpu_p.h
    system/private/cpu_set_p.h
    system/private/block_device_p.h
    system/private/netif_p.h
    system/private/sys_info_p.h
    system/system_monitor.h
    system/system_monitor_thread.h
    system/device_id_cache.h
    system/packet.h
    system/netif.h
    system/netif_info_db.h
    system/netif_monitor.h
    system/netif_monitor_thread.h
    system/netif_packet_capture.h
    system/netif_packet_parser.h
    system/mem.h
    system/cpu.h
    system/cpu_set.h
    system/block_device.h
    system/block_device_info_db.h
    system/device_db.h
    system/sys_info.h
    system/udev.h
    system/udev_device.h
    system/netlink.h
    system/nl_addr.h
    system/nl_hwaddr.h
    system/nl_link.h
    system/wireless.h
    system/diskio_info.h
    system/net_info.h
)
set(CPP_SYSTEM
    system/system_monitor.cpp
    system/system_monitor_thread.cpp
    system/device_id_cache.cpp
    system/netif_monitor.cpp
    system/netif_monitor_thread.cpp
    system/netif_packet_capture.cpp
    system/netif_packet_parser.cpp
    system/device_db.cpp
    system/netif.cpp
    system/netif_info_db.cpp
    system/mem.cpp
    system/cpu.cpp
    system/cpu_set.cpp
    system/block_device.cpp
    system/block_device_info_db.cpp
    system/sys_info.cpp
    system/udev.cpp
    system/udev_device.cpp
    system/netlink.cpp
    system/nl_addr.cpp
    system/nl_hwaddr.cpp
    system/nl_link.cpp
    system/wireless.cpp
    system/diskio_info.cpp
    system/net_info.cpp
)

set(HPP_WM
    wm/wm_atom.h
    wm/wm_connection.h
    wm/wm_info.h
    wm/wm_window_list.h
    wm/wm_window_tree.h
)
set(CPP_WM
    wm/wm_atom.cpp
    wm/wm_connection.cpp
    wm/wm_info.cpp
    wm/wm_window_list.cpp
    wm/wm_window_tree.cpp
)

set(LSCPU
        3rdparty/libsmartcols/src/calculate.c
        3rdparty/libsmartcols/src/cell.c
        3rdparty/libsmartcols/src/column.c
        3rdparty/libsmartcols/src/grouping.c
        3rdparty/libsmartcols/src/init.c
        3rdparty/libsmartcols/src/iter.c
        3rdparty/libsmartcols/src/line.c
        3rdparty/libsmartcols/src/print-api.c
        3rdparty/libsmartcols/src/print.c
        3rdparty/lscpu.c
        3rdparty/libsmartcols/src/symbols.c
        3rdparty/libsmartcols/src/table.c
        3rdparty/libsmartcols/src/version.c
        3rdparty/libsmartcols/src/walk.c
        3rdparty/lscpu-arm.c
        3rdparty/lscpu-cpu.c
        3rdparty/lscpu-cputype.c
        3rdparty/lscpu-dmi.c
        3rdparty/lscpu-topology.c
        3rdparty/lscpu-virt.c
        3rdparty/cpuset.c
        3rdparty/path.c
        3rdparty/strutils.c
        3rdparty/buffer.c
        3rdparty/mbsalign.c
        3rdparty/color-names.c
        3rdparty/jsonwrt.c
        3rdparty/ttyutils.c
        3rdparty/c_strtod.c
        3rdparty/fileutils.c
)
set(LSCPU_INCLUDE
        3rdparty/libsmartcols/src/libsmartcols.h
        3rdparty/libsmartcols/src/smartcolsP.h
        3rdparty/include/all-io.h
        3rdparty/include/encode.h
        3rdparty/include/minix.h
        3rdparty/include/pty-session.h
        3rdparty/include/bitops.h
        3rdparty/include/env.h
        3rdparty/include/monotonic.h
        3rdparty/include/pwdutils.h
        3rdparty/include/blkdev.h
        3rdparty/include/exec_shell.h
        3rdparty/include/namespace.h
        3rdparty/include/randutils.h
        3rdparty/include/buffer.h
        3rdparty/include/exitcodes.h
        3rdparty/include/nls.h
        3rdparty/include/rpmatch.h
        3rdparty/include/canonicalize.h
        3rdparty/include/fileutils.h
        3rdparty/include/optutils.h
        3rdparty/include/selinux-utils.h
        3rdparty/include/caputils.h
        3rdparty/include/fuzz.h
        3rdparty/include/pager.h
        3rdparty/include/sha1.h
        3rdparty/include/carefulputc.h
        3rdparty/include/idcache.h
        3rdparty/include/partx.h
        3rdparty/include/signames.h
        3rdparty/include/cctype.h
        3rdparty/include/ismounted.h
        3rdparty/include/path.h
        3rdparty/include/statfs_magic.h
        3rdparty/include/c.h
        3rdparty/include/iso9660.h
        3rdparty/include/pathnames.h
        3rdparty/include/strutils.h
        3rdparty/include/closestream.h
        3rdparty/include/jsonwrt.h
        3rdparty/include/pidfd-utils.h
        3rdparty/include/strv.h
        3rdparty/include/color-names.h
        3rdparty/include/linux_version.h
        3rdparty/include/plymouth-ctrl.h
        3rdparty/include/swapheader.h
        3rdparty/include/colors.h
        3rdparty/include/list.h
        3rdparty/include/procutils.h
        3rdparty/include/swapprober.h
        3rdparty/include/cpuset.h
        3rdparty/include/loopdev.h
        3rdparty/include/pt-bsd.h
        3rdparty/include/sysfs.h
        3rdparty/include/crc32c.h
        3rdparty/include/mangle.h
        3rdparty/include/pt-gpt-partnames.h
        3rdparty/include/timer.h
        3rdparty/include/crc32.h
        3rdparty/include/match.h
        3rdparty/include/pt-mbr.h
        3rdparty/include/timeutils.h
        3rdparty/include/c_strtod.h
        3rdparty/include/mbsalign.h
        3rdparty/include/pt-mbr-partnames.h
        3rdparty/include/ttyutils.h
        3rdparty/include/debug.h
        3rdparty/include/mbsedit.h
        3rdparty/include/pt-sgi.h
        3rdparty/include/widechar.h
        3rdparty/include/debugobj.h
        3rdparty/include/md5.h
        3rdparty/include/pt-sun.h
        3rdparty/include/xalloc.h
        3rdparty/lscpu.h
        )
set(DMIDECODE_HEADS
            3rdparty/dmidecode/dmidecode.h
            3rdparty/dmidecode/dmioem.h
            3rdparty/dmidecode/dmiopt.h
            3rdparty/dmidecode/dmioutput.h
            3rdparty/dmidecode/types.h
            3rdparty/dmidecode/util.h
            3rdparty/dmidecode/version.h
            3rdparty/dmidecode/vpdopt.h
       )
set(DMIDECODE
            3rdparty/dmidecode/dmidecode.c
            3rdparty/dmidecode/dmioem.c
            3rdparty/dmidecode/dmiopt.c
            3rdparty/dmidecode/dmioutput.c
            3rdparty/dmidecode/util.c
       )

set(DMIDECODE_HEADS
            3rdparty/displayjack/wayland_client.h
       )

set(APP_HPP
    ${CMAKE_HOME_DIRECTORY}/config.h
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
set(LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
    Qt5::X11Extras
    dtkcore
    dtkwidget
    dtkgui
    ${LIB_PCAP}
    ICU::i18n
    ICU::uc
    ${LIB_XCB}
    ${LIB_XEXT}
    ${LIB_ICCCM}
    ${LIB_NL3_LIBRARIES}
    ${LIB_NL3_ROUTE_LIBRARIES}
    ${LIB_UDEV_LIBRARIES}
#    ${DFrameworkDBus_LIBRARIES}   # chinalife
    Dtk::Core
)

if (USE_DEEPIN_WAYLAND)
    if (DWayland_FOUND)
        list(APPEND LIBS DWaylandClient DWaylandServer)
    else()
        list(APPEND LIBS KF5::WaylandClient KF5::WaylandServer)
    endif()
endif()

file(GLOB APP_TS_FILES LIST_DIRECTORIES false translations/*.ts)
qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})
add_custom_target(QMFILES ALL DEPENDS ${APP_QM_FILES})

find_program(POLICY_TS_CONV deepin-policy-ts-convert REQUIRED)
add_custom_target(APP_POLICY_FILE
    ALL
    COMMAND ${POLICY_TS_CONV} ts2policy ${CMAKE_CURRENT_SOURCE_DIR}/translations/policy/${POLICY_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/translations/policy/ ${POLICY_FILE}.tmp
    COMMAND mv ${POLICY_FILE}.tmp ${POLICY_FILE}
)

find_program(DESKTOP_TS_CONV deepin-desktop-ts-convert REQUIRED)
add_custom_target(APP_DESKTOP_FILE
    ALL
    COMMAND ${DESKTOP_TS_CONV} ts2desktop ${CMAKE_CURRENT_SOURCE_DIR}/translations/desktop/${DESKTOP_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/translations/desktop/ ${DESKTOP_FILE}.tmp
    COMMAND mv ${DESKTOP_FILE}.tmp ${DESKTOP_FILE}
)

add_executable(${PROJECT_NAME}
    ${APP_HPP}
    ${APP_CPP}
    ${APP_RESOURCES}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
install(FILES translations/policy/${POLICY_FILE} DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions)
install(FILES translations/desktop/${DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/assets/deepin-system-monitor DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)

#hw机型增加DConfig配置
set(APPID org.deepin.system-monitor)
message("-- DConfig is supported by DTK ${PROJECT_SOURCE_DIR}")
set(configFile ${PROJECT_SOURCE_DIR}/assets/org.deepin.system-monitor.main.json)
if (DEFINED DSG_DATA_DIR)
    message("-- DConfig is supported by DTK")
    dconfig_meta_files(APPID ${APPID} FILES ${configFile})
endif()
