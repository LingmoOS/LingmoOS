find_library(GLIBC_LIB glib-2.0)
include_directories("/usr/include/glib-2.0")
add_library(kylocation SHARED libkylocation.c)
set_target_properties(kylocation PROPERTIES VERSION 2.5.1 SOVERSION 1)
target_link_libraries(kylocation -lglib-2.0 -lgio-2.0)
add_executable(kylocation-test test/kylocation-test.c)
target_link_libraries(kylocation-test kylocation)

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
install(TARGETS kylocation
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kylocation
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkylocation.h
        DESTINATION include/lingmosdk/lingmosdk-system)
