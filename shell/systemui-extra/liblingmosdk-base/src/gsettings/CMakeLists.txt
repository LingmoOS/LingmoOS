aux_source_directory(. SOURCESCODE)
include(FindPkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0 glib-2.0)
include_directories(${GTK3_INCLUDE_DIRS})

link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_library(kygsetting SHARED ${SOURCESCODE})
set_target_properties(kygsetting PROPERTIES VERSION 2.5.0 SOVERSION 1)

add_executable(test-kygsetting test/test-gsetting.c)
target_link_libraries(kygsetting ${GTK3_LIBRARIES})
target_link_libraries(test-kygsetting kygsetting)

# install(TARGETS kygsetting
#         DESTINATION lib/lingmosdk/lingmosdk-base)

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
install(TARGETS kygsetting 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkygsetting.h
        DESTINATION include/lingmosdk/lingmosdk-base)
