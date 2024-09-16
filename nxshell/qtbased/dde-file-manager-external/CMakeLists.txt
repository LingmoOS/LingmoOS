cmake_minimum_required(VERSION 3.10)

project(dde-file-manager-external)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")

#安全加固
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Werror=return-type")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
    add_definitions(-D ARCH_MIPSEL)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(DtkCMake REQUIRED)
#if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)
# TODO: control it by CI. (Automation Testing)
add_definitions(-DENABLE_TESTING)

#Indentify the version
if (${DEEPIN_OS_VERSION} MATCHES "23")
    add_definitions(-DCOMPILE_ON_V23)
    set(COMPLIE_ON_V23 TRUE)
    message("COMPILE ON v23")
else()
    add_definitions(-DCOMPILE_ON_V20)
    set(COMPLIE_ON_V20 TRUE)
    message("COMPILE ON v20")
endif()

include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

# defines plugins install dir
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message("Build type:"${CMAKE_BUILD_TYPE})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TESTING ON)
endif()

# sub directories
add_subdirectory(src/dde-control-center-plugins)

# test
# message(STATUS "Enable testing: ${BUILD_TESTING}")
# if(BUILD_TESTING)
#     enable_testing()
#     add_subdirectory(tests)
# endif()
