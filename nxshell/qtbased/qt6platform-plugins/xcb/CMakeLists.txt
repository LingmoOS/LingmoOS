# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

project(dxcb)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XcbQpa X11Extras EdidSupport XkbCommonSupport)
else()
    find_package(Qt6 REQUIRED COMPONENTS OpenGL XcbQpaPrivate)
endif()

# NOTE(sbw): 禁止语法树上的 vrp 优化，-O2/-O3 默认开启，会导致测试虚析构函数 HOOK 失败
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-tree-vrp" CACHE STRING "disable vrp optimization" FORCE)

add_definitions(-DDXCB_VERSION=\"${DTK_VERSION}\")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DQT_NO_DEBUG_OUTPUT=TRUE)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/xcb.cmake)
include(${CMAKE_SOURCE_DIR}/src/src.cmake)

add_library(${PROJECT_NAME} SHARED ${GLOBAL_HEADERS} ${GLOBAL_SOURCES} ${xcb_SRC} ${DBUS_INTERFACE_XMLS})

include(${CMAKE_CURRENT_LIST_DIR}/linux.cmake)

set(EnableCov CACHE BOOL FALSE)
if (EnableCov)
    find_package(DtkBuildHelper REQUIRED)
    dtk_setup_code_coverage(${PROJECT_NAME})
endif()

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::XcbQpa
        Qt5::EdidSupport
        Qt5::EdidSupportPrivate
        Qt5::XkbCommonSupport
        Qt5::XkbCommonSupportPrivate
        Qt5::X11Extras
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGL Qt6::OpenGLPrivate Qt6::XcbQpaPrivate)
endif()

if(NOT INSTALL_PATH)
    set(INSTALL_PATH ${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/plugins/platforms)
endif()
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_PATH})
