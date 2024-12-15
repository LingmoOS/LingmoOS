# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-notification SHARED
    notificationcontainment.h
    notificationcontainment.cpp
)

target_link_libraries(ds-notification PRIVATE
    ocean-shell-frame
)

add_library(ds-notification-shared SHARED
    ${CMAKE_SOURCE_DIR}/panels/notification/common/notifyentity.h
    ${CMAKE_SOURCE_DIR}/panels/notification/common/notifyentity.cpp
    ${CMAKE_SOURCE_DIR}/panels/notification/common/dbaccessor.h
    ${CMAKE_SOURCE_DIR}/panels/notification/common/dbaccessor.cpp
    ${CMAKE_SOURCE_DIR}/panels/notification/common/notifysetting.h
    ${CMAKE_SOURCE_DIR}/panels/notification/common/notifysetting.cpp
)

set_target_properties(ds-notification-shared PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)
target_include_directories(ds-notification-shared PUBLIC
    ${CMAKE_SOURCE_DIR}/panels/notification/common
)
target_link_libraries(ds-notification-shared PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Dtk${DTK_VERSION_MAJOR}::Core
)

install(TARGETS ds-notification-shared DESTINATION "${LIB_INSTALL_DIR}")
ds_install_package(PACKAGE org.lingmo.ds.notification TARGET ds-notification)

add_subdirectory(plugin)
add_subdirectory(bubble)
add_subdirectory(center)
add_subdirectory(osd)
add_subdirectory(server)
