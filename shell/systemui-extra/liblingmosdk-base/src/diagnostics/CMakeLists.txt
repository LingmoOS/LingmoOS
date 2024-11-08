cmake_minimum_required(VERSION 3.5)

project(kydiagnostics LANGUAGES CXX)

set(DIAGNOSTICS_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} SHARED)
set_target_properties(kydiagnostics PROPERTIES VERSION 2.5.0 SOVERSION 1)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(PkgConfig REQUIRED)

pkg_check_modules(OPENSSL openssl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})

pkg_check_modules(DBUS-1 dbus-1)
target_include_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARIES})


set(SRCS 
    "${DIAGNOSTICS_TOP_DIR}/libkydiagnostics.cpp"
    "${DIAGNOSTICS_TOP_DIR}/buriedpoint.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${DIAGNOSTICS_TOP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

# install(TARGETS ${PROJECT_NAME} DESTINATION lib/lingmosdk/lingmosdk-base)

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
install(TARGETS ${PROJECT_NAME} 
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
