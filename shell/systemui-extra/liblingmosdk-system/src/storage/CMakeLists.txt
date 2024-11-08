add_library(kystorage SHARED libkystorage.c)
set_target_properties(kystorage PROPERTIES VERSION 2.5.1 SOVERSION 1)
target_link_libraries(kystorage kylog kypackage magic)

add_executable(kystorage-test test/kystorage-test.c)
target_link_libraries(kystorage-test kystorage)

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
install(TARGETS kystorage
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkystorage.h
        DESTINATION include/lingmosdk/lingmosdk-system)