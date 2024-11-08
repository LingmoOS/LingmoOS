aux_source_directory(. SOURCESCODE)
add_library(kyconf SHARED ${SOURCESCODE})
set_target_properties(kyconf PROPERTIES VERSION 2.5.0 SOVERSION 1)
add_executable(kyconf-test-struct test/test_structlist.c)
target_link_libraries(kyconf-test-struct kyconf)

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu/")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/")
endif()
# 安装库文件  
install(TARGETS kyconf 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS kyconf
#         DESTINATION lib/lingmosdk/lingmosdk-base)

# install(FILES libkyconf.h
#         DESTINATION include/lingmosdk/lingmosdk-base)
