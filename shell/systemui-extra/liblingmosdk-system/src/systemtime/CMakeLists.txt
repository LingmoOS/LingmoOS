FILE(GLOB SRC_LIST_C ${PROJECT_SOURCE_DIR}/src/*.c)
include(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED
                dbus-1
		glib-2.0
		dbus-glib-1
                )
# message("pkg cflags: ${PKGS_CFLAGS}")
# message ("pkg include dirs: ${PKGS_INCLUDE_DIRS}")
find_library(GLIBC_LIB glib-2.0)
# message("library :${GLIBC_LIB}")
find_library(DBUS_LIB dbus-1)
find_library(DBUS_GLIB_LIB dbus-glib-1)
find_library(THREAD_LIB pthread)
# link_directories(/usr/lib/lingmosdk/lingmosdk-base/)
include_directories(${PKGS_INCLUDE_DIRS} /usr/lib/aarch64-linux-gnu/glib-2.0/include /usr/include/glib-2.0)
# SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ADD_EXECUTABLE(systime m_systime.c)
target_link_libraries(systime ${GLIBC_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB} ${THREAD_LIB} ${TIMER_LIST_C} kytimer kylog kyconf)

aux_source_directory(. SOURCESCODE)
include_directories(.)
add_library(kydate SHARED ./libkydate.c)
set_target_properties(kydate PROPERTIES VERSION 2.5.1 SOVERSION 1)
ADD_EXECUTABLE(kydate_test test/kydate_test.c)
target_link_libraries(kydate ${GLIBC_LIB} kyconf kylog)
target_link_libraries(kydate_test kydate)
ADD_EXECUTABLE(sdk-date date.c)
target_link_libraries(sdk-date ${GLIBC_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB} ${THREAD_LIB} kyconf)

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
install(TARGETS kydate
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kydate
#         DESTINATION lib/lingmosdk/lingmosdk-system)

install(FILES libkydate.h
        DESTINATION include/lingmosdk/lingmosdk-system)
