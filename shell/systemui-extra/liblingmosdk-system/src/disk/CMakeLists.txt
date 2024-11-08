aux_source_directory(. SOURCESCODE)
add_library(kydiskinfo SHARED ${SOURCESCODE})
set_target_properties(kydiskinfo PROPERTIES VERSION 2.5.1 SOVERSION 1)
add_executable(test-getdiskinfo test/getdiskinfo.c)
add_executable(test-getdisklist test/getdisklist.c)
find_library(UDEV_LIB udev)
find_library(DBUS_LIB dbus-1)
target_link_libraries(kydiskinfo blkid kylog pthread systemd kyconf hd ${UDEV_LIB})
target_link_libraries(test-getdiskinfo kydiskinfo)
target_link_libraries(test-getdisklist kydiskinfo)
# target_link_libraries(test-getdiskinfo kydiskinfo blkid kylog pthread systemd kyconf ${UDEV_LIB})
# target_link_libraries(test-getdisklist kydiskinfo blkid kylog pthread systemd kyconf ${UDEV_LIB})

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
install(TARGETS kydiskinfo 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS kydiskinfo
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkydiskinfo.h
        DESTINATION include/lingmosdk/lingmosdk-system)
