aux_source_directory(. SOURCESCODE)
include_directories(.)
add_library(kyrtinfo SHARED ${SOURCESCODE})
set_target_properties(kyrtinfo PROPERTIES VERSION 2.5.1 SOVERSION 1)
add_executable(kyrtinfo-test test/kyrtinfo-test.c)
add_executable(kyprocess-test test/kyprocess-test.c)
target_link_libraries(kyrtinfo kylog)
target_link_libraries(kyrtinfo-test kyrtinfo) 
target_link_libraries(kyprocess-test kyrtinfo)

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
install(TARGETS kyrtinfo
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kyrtinfo
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkyrtinfo.h
        DESTINATION include/lingmosdk/lingmosdk-system)
        
install(FILES libkyprocess.h
        DESTINATION include/lingmosdk/lingmosdk-system)
