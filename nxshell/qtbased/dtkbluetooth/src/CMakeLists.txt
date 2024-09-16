# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later


file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)

add_library(${LIB_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(USE_FAKE_INTERFACE)
endif()

target_include_directories(${LIB_NAME}
PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/DBus>
INTERFACE
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_directories(${LIB_NAME}
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${LIBRARY_INSTALL_DIR}>
)

target_compile_definitions(${LIB_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_link_libraries(${LIB_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Bluetooth
    Dtk${DTK_VERSION_MAJOR}::Core
)

set_target_properties(${LIB_NAME} PROPERTIES
    EXPORT_NAME Bluetooth
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${INCLUDE_FILES}")

install(TARGETS ${LIB_NAME} EXPORT ${DtkBluetooth}Targets
    LIBRARY DESTINATION ${LIBRARY_INSATLL_DIR}
    PUBLIC_HEADER DESTINATION ${INCLUDE_INSTALL_DIR}
)

install(EXPORT ${DtkBluetooth}Targets
    FILE ${DtkBluetooth}Targets.cmake
    NAMESPACE Dtk${DTK_VERSION_MAJOR}::
    DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
)
