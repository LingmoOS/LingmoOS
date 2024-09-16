cmake_minimum_required(VERSION 3.13)

project(dde-cooperation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fPIE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-need")

    #安全加固
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

    #安全加固
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
        add_definitions(-D ARCH_MIPSEL)
    endif()
endif()

if(WIN32)
    # windows7 support
    add_definitions(-D_WIN32_WINNT=0x0601)

    # windeployqt 依赖
    include (cmake/qt-helper.cmake)

    # windows 并行编译选项
    cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)
endif()

include(GNUInstallDirs)

option(FPIC "build with -fPIC" FALSE)

if(MSVC)
    add_compile_options(/fp:fast /EHsc)
    add_link_options(/SAFESEH:NO)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
else()
    add_compile_options(-Wall -O2 -g -Wno-sign-compare -Wno-strict-aliasing)
    set(FPIC TRUE)
endif()

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

include (cmake/version.cmake)

message("   >>> LIB_INSTALL_DIR  ${LIB_INSTALL_DIR}")
message("   >>> COMPILE ON SYSTEM: ${CMAKE_SYSTEM}")

include_directories(include)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

if (WIN32)
    set(DEPLOY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output")
    message("   >>> DEPLOY_OUTPUT_DIRECTORY  ${DEPLOY_OUTPUT_DIRECTORY}")

    # windows runtime output defined
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY})
#    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY})
#    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY})

    file(GLOB INNO_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/inno/scripts)
    file(COPY ${INNO_SCRIPTS} DESTINATION ${CMAKE_BINARY_DIR})
endif()

# 如果未设置CMAKE_BUILD_TYPE，则将其设置为Release
message("   >>> CMAKE_BUILD_TYPE  ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
   message("Cmake build type is not defined!")
   set(CMAKE_BUILD_TYPE "Debug")
endif()

# 后台进程安装目录
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(EXECUTABLE_INSTALL_DIR "")
else()
  set(EXECUTABLE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/libexec/${PROJECT_NAME}")
endif()

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# 检查并设置环境变量
if(NOT DEFINED ENV{QT_SELECT})
    set(ENV{QT_SELECT} "qt${QT_VERSION_MAJOR}")
endif()
message("   >>> Build with Qt version: $ENV{QT_SELECT}")

if (QT_VERSION_MAJOR EQUAL "6")
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

# 引入翻译生成
include(translation-generate)

# header-only libraries: jwt, picojson
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include)

# Modules
add_subdirectory("modules")

if(MSVC)
    add_compile_options(/utf-8)
endif()
add_subdirectory(src)
