# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 显示编译过程
set(CMAKE_VERBOSE_MAKEFILE ON)

# 设置工程名字
project(dde-grand-search)

# Install settings
include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need")

#安全加固
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -z now")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
endif()

#Indentify the version
find_package(DtkCMake REQUIRED)

if (1 OR ${DEEPIN_OS_VERSION} MATCHES "23")
    add_definitions(-DCOMPILE_ON_V23)
    set(COMPLIE_ON_V23 TRUE)
    message("COMPILE ON v23")
else()
    add_definitions(-DCOMPILE_ON_V20)
    set(COMPLIE_ON_V20 TRUE)
    message("COMPILE ON v20")
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/environments.h.in ${CMAKE_SOURCE_DIR}/src/environments.h @ONLY)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/src/tools)

# 平台库目录
set(LIB_PLATFORM_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
message("LIB_PLATFORM_DIR: ${LIB_PLATFORM_DIR}")

#全局文件
FILE(GLOB GLOBAL_SRC "./src/global/*.h" "./src/global/*.cpp" "./src/global/accessibility/*.h")

# 国际化
if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# 定义前端可执行程序名称
set(SEARCH_BIN_NAME dde-grand-search)

# 定义前端应用库目录
set(SEARCH_LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${SEARCH_BIN_NAME}")

# 定义前端插件目录
set(SEARCH_PLUGIN_BASE_DIR "${SEARCH_LIB_BASE_DIR}/plugins")

# 定义前端预览插件目录
set(PLUGIN_PREVIEW_DIR "${SEARCH_PLUGIN_BASE_DIR}/preview")
add_definitions(-DPLUGIN_PREVIEW_DIR="${PLUGIN_PREVIEW_DIR}")

# 定义后端库名称
set(DAEMON_LIB_NAME libdde-grand-search-daemon)

# 定义后端可执行程序名称
set(DAEMON_BIN_NAME dde-grand-search-daemon)

# 定义后端应用库目录
set(DAEMON_LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${DAEMON_BIN_NAME}")

# 定义后端插件目录
set(DAEMON_PLUGIN_DIR "${DAEMON_LIB_BASE_DIR}/plugins")

# 定义3rdparty目录
set(PROJECT_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/3rdparty")

# 定义后端扩展搜索插件目录
set(PLUGIN_SEARCHER_DIR "${DAEMON_PLUGIN_DIR}/searcher")
add_definitions(-DPLUGIN_SEARCHER_DIR="${PLUGIN_SEARCHER_DIR}")

add_subdirectory(src/grand-search)
add_subdirectory(src/libgrand-search-daemon)
add_subdirectory(src/grand-search-daemon)
add_subdirectory(src/grand-search-dock-plugin)
add_subdirectory(src/preview-plugin)
add_subdirectory(src/tools/semanticparser)
add_subdirectory(src/tools/luceneengine)
