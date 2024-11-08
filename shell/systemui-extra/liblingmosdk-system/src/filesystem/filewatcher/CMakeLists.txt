set (CMAKE_AUTOMOC ON)

aux_source_directory(. SOURCECODE)

find_package(Qt5Core)
include_directories(${Qt5Core_INCLUDE_DIRS})
add_library(kyfilewatcher SHARED ${SOURCECODE})
set_target_properties(kyfilewatcher PROPERTIES VERSION 2.5.1 SOVERSION 1)
add_executable(kyfilewatcher-test test/kyfilewatcher-test.cpp)
target_link_libraries(kyfilewatcher kylog kyconf systemd pthread ${Qt5Core_LIBRARIES})
target_link_libraries(kyfilewatcher-test kyfilewatcher)

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
install(TARGETS kyfilewatcher 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS kyfilewatcher
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkyfilewatcher.hpp
        DESTINATION include/lingmosdk/lingmosdk-system)

install(FILES libkyfilewatcher_global.hpp
        DESTINATION include/lingmosdk/lingmosdk-system)
