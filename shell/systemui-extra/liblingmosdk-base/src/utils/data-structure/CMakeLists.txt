aux_source_directory(linklist/skip_linklist SOURCESCODESTRUCT)
add_library(kydatastruct SHARED ${SOURCESCODESTRUCT})
set_target_properties(kydatastruct PROPERTIES VERSION 2.3.0 SOVERSION 1)
add_subdirectory(linklist)

include_directories(linklist)
include_directories(linklist/skip_linklist)

add_executable(test-delete linklist/skip_linklist/test/delete_test.c)
add_executable(test-insert linklist/skip_linklist/test/insert_test.c)
add_executable(test-search linklist/skip_linklist/test/search_test.c)
target_link_libraries(test-delete kydatastruct)
target_link_libraries(test-insert kydatastruct)
target_link_libraries(test-search kydatastruct)

# install(TARGETS kydatastruct
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
install(TARGETS kydatastruct 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
