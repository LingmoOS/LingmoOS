aux_source_directory(. CODE)

add_library(kyrealtime SHARED ${CODE})

set_target_properties(kyrealtime PROPERTIES VERSION 2.5.1 SOVERSION 1)

find_library(DBUS_LIB dbus-1)
add_executable(kyrealtime-test test/kyrealtime-test.c)
target_link_libraries(kyrealtime ${DBUS_LIB})
target_link_libraries(kyrealtime-test kyrealtime)

add_executable(realtimeinfo-test test/realtimeinfo-test.cpp)
target_link_libraries(realtimeinfo-test kyrealtime)

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
install(TARGETS kyrealtime
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kyrealtime
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkyrealtimeinfo.h
        DESTINATION include/lingmosdk/lingmosdk-system)
