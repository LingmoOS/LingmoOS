cmake_minimum_required(VERSION 3.5)

project(kypowermanagement LANGUAGES CXX)

set(POWERMANAGEMENT_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} SHARED)
set_target_properties(kypowermanagement PROPERTIES VERSION 2.5.1 SOVERSION 1)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS-1 dbus-1)
target_include_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARIES})

pkg_check_modules(KYLOG lingmosdk-log)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYLOG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARIES})

pkg_check_modules(KYGSETTING lingmosdk-gsetting)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYGSETTING_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYGSETTING_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYGSETTING_LIBRARIES})

set(SRCS 
    "${POWERMANAGEMENT_TOP_DIR}/lockscreen.cpp"
    "${POWERMANAGEMENT_TOP_DIR}/libkylockscreen.cpp"
    "${POWERMANAGEMENT_TOP_DIR}/libkypowermanager.c")

add_executable(kypowermanager-test test/kypowermanager-test.c)


target_include_directories(${PROJECT_NAME} PRIVATE ${POWERMANAGEMENT_TOP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})
target_link_libraries(kypowermanager-test ${PROJECT_NAME}) 

pkg_check_modules(GTK3 REQUIRED gtk+-3.0 glib-2.0)
include_directories(${GTK3_INCLUDE_DIRS})

link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

find_library(THREAD_LIB pthread)
find_library(DBUS_LIB dbus-1)
find_library(DBUS_GLIB_LIB dbus-glib-1)
ADD_EXECUTABLE(sdk-powermanager powermanager.c)
target_link_libraries(sdk-powermanager ${GTK3_LIBRARIES} ${THREAD_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB} kylog)

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
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS ${PROJECT_NAME} DESTINATION lib/lingmosdk/lingmosdk-system)
