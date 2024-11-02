project(kyimageproc)
aux_source_directory(. SOURCESCODE)
include_directories(.)
add_library(kyimageproc SHARED ${SOURCESCODE})
set_target_properties(kyimageproc PROPERTIES VERSION 2.5.1 SOVERSION 1)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LINGMOSDKLOG lingmosdk-log)
target_include_directories(kyimageproc PRIVATE ${LINGMOSDKLOG_INCLUDE_DIRS})
target_link_directories(kyimageproc PRIVATE ${LINGMOSDKLOG_LIBRARY_DIRS})
target_link_libraries(kyimageproc PRIVATE ${LINGMOSDKLOG_LIBRARIES} dl)

set(PREFIX "")
set(LIBS_INSTALL_DIR "${PREFIX}/usr/lib/lingmosdk/lingmosdk-system")
set(HEADERS_INSTALL_DIR "${PREFIX}/usr/include/lingmosdk/lingmosdk-system/")

# install(TARGETS kyimageproc LIBRARY DESTINATION ${LIBS_INSTALL_DIR})

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
install(TARGETS kyimageproc
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkyimageproc.h DESTINATION ${HEADERS_INSTALL_DIR})
