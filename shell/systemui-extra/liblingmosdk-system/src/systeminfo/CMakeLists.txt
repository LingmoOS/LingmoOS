aux_source_directory(. SOURCESCODE)
include_directories(.)
include_directories(/usr/include/cjson/)
find_library(GLIBC_LIB glib-2.0)
find_library(DBUS_LIB dbus-1)
find_library(DBUS_GLIB_LIB dbus-glib-1)
find_package(PkgConfig)
add_library(kysysinfo SHARED libkysysinfo.c)
set_target_properties(kysysinfo PROPERTIES VERSION 2.5.1 SOVERSION 1)
add_executable(kysysinfo-test test/kysysinfo_test.c)
target_link_libraries(kysysinfo dl kylog systemd kyconf X11 Xrandr libcjson.so pci kmod ${GLIBC_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB})
target_link_libraries(kysysinfo-test kysysinfo libcjson.so)
# target_link_libraries(kysysinfo-test kysysinfo lingmo-activation kylog systemd kyconf ${GLIBC_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB})

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
install(TARGETS kysysinfo
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kysysinfo
#         DESTINATION lib/lingmosdk/lingmosdk-system)
        
install(FILES libkysysinfo.hpp
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkysysinfo.h
        DESTINATION include/lingmosdk/lingmosdk-system)
