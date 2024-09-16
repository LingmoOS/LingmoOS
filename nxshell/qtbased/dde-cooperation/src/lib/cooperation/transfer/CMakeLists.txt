cmake_minimum_required(VERSION 3.13)

# enable qt build support
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set build as share library
set(BUILD_SHARED_LIBS ON)

project(cooperation-transfer VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CORE_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../core")

# 插件相关的固定内容，或者公共头文件源文件，添加到这里。
FILE(GLOB PLUGIN_FILES
    "${CMAKE_SOURCE_DIR}/src/base/baseutils.h"
    "${CMAKE_SOURCE_DIR}/src/base/baseutils.cpp"
    "${CMAKE_SOURCE_DIR}/src/singleton/*.h"
    "${CMAKE_SOURCE_DIR}/src/singleton/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/configs/settings/*.h"
    "${CMAKE_SOURCE_DIR}/src/configs/settings/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/log.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/qtcompat.h"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.h"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/transferplugin.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/transferplugin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper/*.cpp"
    "${CORE_UI_DIR}/global_defines.h"
    "${CORE_UI_DIR}/gui/*.h"
    "${CORE_UI_DIR}/gui/*.cpp"
    "${CORE_UI_DIR}/gui/dialogs/*.h"
    "${CORE_UI_DIR}/gui/dialogs/*.cpp"
    "${CORE_UI_DIR}/gui/widgets/*.h"
    "${CORE_UI_DIR}/gui/widgets/*.cpp"
    "${CORE_UI_DIR}/gui/utils/*"
    "${CORE_UI_DIR}/discover/deviceinfo.h"
    "${CORE_UI_DIR}/discover/deviceinfo.cpp"
    "${CORE_UI_DIR}/utils/*.h"
    "${CORE_UI_DIR}/utils/*.cpp"
    )

# 在这里添加业务代码
if (CMAKE_SYSTEM MATCHES "Windows")
    FILE(GLOB BUSS_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/win/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/win/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/net/win/*.cpp"
    )
#    find_package(Qt5 COMPONENTS Sql REQUIRED)

   # 从定位输出目录
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY}/dde-cooperation)
elseif (CMAKE_SYSTEM MATCHES "Linux")
    FILE(GLOB BUSS_FILES
        "${CMAKE_SOURCE_DIR}/src/base/reportlog/*.h"
        "${CMAKE_SOURCE_DIR}/src/base/reportlog/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/base/reportlog/*/*.h"
        "${CMAKE_SOURCE_DIR}/src/base/reportlog/*/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/configs/dconfig/*.h"
        "${CMAKE_SOURCE_DIR}/src/configs/dconfig/*.cpp"
        "${CORE_UI_DIR}/gui/linux/*.h"
        "${CORE_UI_DIR}/gui/linux/*.cpp"
        "${CORE_UI_DIR}/net/linux/*.cpp"
    )
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
else()
    message (FATAL_ERROR "not support on ${CMAKE_SYSTEM} yet.")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Network DBus Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network DBus Concurrent)

# 根据Qt版本选择合适的命令
if (QT_VERSION VERSION_GREATER_EQUAL "6.0")
    qt_add_resources(RESOURCE_FILES ${CORE_UI_DIR}/gui/resources/resources.qrc)
else()
    qt5_add_resources(RESOURCE_FILES ${CORE_UI_DIR}/gui/resources/resources.qrc)
endif()

list(APPEND LINKLIBS cpplogging)

add_library(${PROJECT_NAME}
    ${PLUGIN_FILES}
    ${BUSS_FILES}
    ${BARRIER_CONFIG_FILES}
    ${RESOURCE_FILES}
)

include(GNUInstallDirs)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_BUILD_SHARED)
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
endif()

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CORE_UI_DIR}
)

# 公共依赖库，添加到这里
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    CuteIPC
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${LINKLIBS}
)

# 平台相关的库，添加到这里
if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
endif()

#install library file
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR})
