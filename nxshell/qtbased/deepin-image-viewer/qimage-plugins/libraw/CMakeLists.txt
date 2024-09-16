project(xraw VERSION 0.1.0)

set(CMAKE_AUTOMOC ON)
set(CMD_NAME xraw)

ADD_DEFINITIONS(-DTORRENT_LIBRARY)
ADD_DEFINITIONS(-DNO_FREETYPE)
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ADD_DEFINITIONS(-DHCPP_LEGACY_API)
ADD_DEFINITIONS(-D_LIBXRAW_)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}Gui REQUIRED)

pkg_check_modules(RAW REQUIRED libraw)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# 加速编译优化参数
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z noexecstack")
    SET(CMAKE_EXE_LINKER_FLAGS "-pie")
endif()

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include(GNUInstallDirs)
include_directories(${PROJECT_BINARY_DIR})

list(APPEND SRCS
    main.cpp
    rawiohandler.cpp
    datastream.cpp)

add_library(${CMD_NAME} SHARED ${SRCS})
target_include_directories(xraw PUBLIC ${RAW_INCLUDE_DIRS})
target_link_libraries(${CMD_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    raw)
set_target_properties(${CMD_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)

# Install the image format plugin
set(PLUGIN_INSTALL_BASE_DIR ${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/plugins CACHE STRING "Plugin install base path")
install(TARGETS ${CMD_NAME} DESTINATION ${PLUGIN_INSTALL_BASE_DIR}/imageformats)
