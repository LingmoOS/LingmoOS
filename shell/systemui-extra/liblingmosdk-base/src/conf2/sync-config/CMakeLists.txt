add_executable(sync-config-system system.c sync-dbus-server.c gdbus/libsyncconfig.c)
target_link_libraries(sync-config-system ${GLIB_LIBRARIES} ${GIO_LIBRARIES} kyconf2 kylog)

add_executable(sync-config-session session.c sync-dbus-server.c gdbus/libsyncconfig.c)
target_link_libraries(sync-config-session ${GLIB_LIBRARIES} ${GIO_LIBRARIES} kyconf2 kylog)

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

# 安装文件  
install(TARGETS sync-config-session
        DESTINATION /usr/bin)

install(TARGETS sync-config-system
        DESTINATION /usr/bin)