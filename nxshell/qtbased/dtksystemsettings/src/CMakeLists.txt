# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_library(${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
)

target_compile_definitions(${LIB_NAME} PRIVATE "VERSION=${CMAKE_PROJECT_VERSION}")

# Set include directories for target itself and targets in the build tree(i.e. in this project, some other targets) that linked to this target
target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/accounts>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/login>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/power>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/rfmanager>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/systemtime>
    $<BUILD_INTERFACE:${DtkCore_INCLUDE_DIRS}>
)

# Set include directory only for external targets linked to this target(i.e. downstream user)
target_include_directories(${LIB_NAME} INTERFACE
    $<INSTALL_INTERFACE:${DTK_INCLUDE_INSTALL_DIR}>
)

add_subdirectory(accounts)
add_subdirectory(login)
add_subdirectory(power)
add_subdirectory(rfmanager)
add_subdirectory(systemtime)

target_link_libraries(${LIB_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_LIBRARIES}
    crypt
)

set_target_properties(${LIB_NAME} PROPERTIES
    EXPORT_NAME SystemSettings
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}"
)

install(TARGETS ${LIB_NAME} EXPORT ${DtkSystemSettings}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${DTK_INCLUDE_INSTALL_DIR}
)

install(EXPORT ${DtkSystemSettings}Targets
    FILE ${DtkSystemSettings}Targets.cmake
    NAMESPACE Dtk${VERSION_SUFFIX}::
    DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
)
