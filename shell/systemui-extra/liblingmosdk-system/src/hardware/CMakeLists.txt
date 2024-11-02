include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0 libnl-3.0)
include_directories(${GLIB_INCLUDE_DIRS})

pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-genl-3.0 libnl-route-3.0)
include_directories(${LIBNL_INCLUDE_DIRS})

pkg_check_modules(LIBNM REQUIRED libnm)
include_directories(${LIBNM_INCLUDE_DIRS})

set(HARDWARE_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(hwcode 
    "${HARDWARE_TOP_DIR}/libkybios.h"
    "${HARDWARE_TOP_DIR}/libkybios.c"
    "${HARDWARE_TOP_DIR}/libkyboard.h"
    "${HARDWARE_TOP_DIR}/libkyboard.c"
    "${HARDWARE_TOP_DIR}/libkycpu.h"
    "${HARDWARE_TOP_DIR}/libkycpu.c"
    "${HARDWARE_TOP_DIR}/libkydisplay.h"
    "${HARDWARE_TOP_DIR}/libkydisplay.c"
    "${HARDWARE_TOP_DIR}/libkync.h"
    "${HARDWARE_TOP_DIR}/libkync.c"
    "${HARDWARE_TOP_DIR}/libkyprinter.h"
    "${HARDWARE_TOP_DIR}/libkyprinter.c"
    "${HARDWARE_TOP_DIR}/libkyusb.h"
    "${HARDWARE_TOP_DIR}/libkyusb.c")


set(bluetoothcode 
    "${HARDWARE_TOP_DIR}/libkybluetooth.h"
    "${HARDWARE_TOP_DIR}/libkybluetooth.c")

set(edidcode 
    "${HARDWARE_TOP_DIR}/libkyedid.h"
    "${HARDWARE_TOP_DIR}/libkyedid.c")

set(fancode 
    "${HARDWARE_TOP_DIR}/libkyfan.h"
    "${HARDWARE_TOP_DIR}/libkyfan.c")

set(hwinfocode 
    "${HARDWARE_TOP_DIR}/libkyhw.h"
    "${HARDWARE_TOP_DIR}/libkyhw.c")

add_library(kyhw SHARED ${hwcode})
set_target_properties(kyhw PROPERTIES VERSION 2.5.1 SOVERSION 1)

add_library(kybluetooth SHARED ${bluetoothcode})
set_target_properties(kybluetooth PROPERTIES VERSION 2.5.1 SOVERSION 1)

add_library(kyedid SHARED ${edidcode})
set_target_properties(kyedid PROPERTIES VERSION 2.5.1 SOVERSION 1)

add_library(kyfan SHARED ${fancode})
set_target_properties(kyfan PROPERTIES VERSION 2.5.1 SOVERSION 1)

add_library(kyhwinfo SHARED ${hwinfocode})
set_target_properties(kyhwinfo PROPERTIES VERSION 2.5.1 SOVERSION 1)

add_executable(kync-test test/kync-test.c)
add_executable(kycpu-test test/kycpu-test.c)
add_executable(kyprinterlist-test test/kyprinterlist-test.c)
add_executable(kyprinterprint-test test/kyprinterprint-test.c)
add_executable(kyusb-test test/kyusb-test.c)
add_executable(kybios-test test/kybios-test.c)
add_executable(kyboard-test test/kyboard-test.c)
add_executable(kydisplay-test test/kydisplay-test.c)
add_executable(kyedid-test test/kyedid-test.c)
add_executable(kybluetooth-test test/kybluetooth-test.c)
add_executable(kyfan-test test/kyfan-test.c)
add_executable(kyhw-test test/kyhw-test.c)
# 额外链接的库在这一行
find_library(DBUS_LIB dbus-1)
target_link_libraries(kyhw kylog kyconf pthread systemd cups curl udev usb-1.0 ${GLIB_LIBRARIES} ${DBUS_LIB} ${LIBNL_LIBRARIES} ${LIBNM_LIBRARIES})
target_link_libraries(kybluetooth bluetooth hd gobject-2.0 kylog ${DBUS_LIB})
target_link_libraries(kyedid kylog m X11 Xrandr hd)
target_link_libraries(kyfan sensors)
target_link_libraries(kyhwinfo kylog hd pci dbus-glib-1)
# target_link_libraries(kyhw kylog kyconf pthread systemd cups curl udev X11 Xrandr bluetooth sensors hd pci gobject-2.0 dbus-glib-1 ${GLIB_LIBRARIES})
target_link_libraries(kyprinterprint-test kyhw)
target_link_libraries(kyprinterlist-test kyhw)
target_link_libraries(kync-test kyhw)
target_link_libraries(kycpu-test kyhw)
target_link_libraries(kyusb-test kyhw)
target_link_libraries(kybios-test kyhw)
target_link_libraries(kyboard-test kyhw)
target_link_libraries(kydisplay-test kyhw)
target_link_libraries(kyedid-test kyedid)
target_link_libraries(kybluetooth-test kybluetooth)
target_link_libraries(kyfan-test kyfan)
target_link_libraries(kyhw-test kyhwinfo)

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu")
endif()
# 安装库文件  
install(TARGETS kyhw 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS kybluetooth 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS kyedid 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS kyfan 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS kyhwinfo 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS kyhw
#         DESTINATION lib/lingmosdk/lingmosdk-system)

# install(TARGETS kybluetooth
#         DESTINATION lib/lingmosdk/lingmosdk-system)

# install(TARGETS kyedid
#         DESTINATION lib/lingmosdk/lingmosdk-system)

# install(TARGETS kyfan
#         DESTINATION lib/lingmosdk/lingmosdk-system)

# install(TARGETS kyhwinfo
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkync.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkycpu.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyprinter.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkybios.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyboard.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyusb.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkydisplay.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyedid.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkybluetooth.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyfan.h
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyhw.h
        DESTINATION include/lingmosdk/lingmosdk-system)
