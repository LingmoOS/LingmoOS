# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(OSD_Name dde-osd)

add_executable(${OSD_Name}
    main.cpp
)

target_link_libraries(${OSD_Name} PRIVATE
    dde-osd-shared
    session-ui-dbus-shared
    ${Test_Libraries}
)

## bin
install(TARGETS ${OSD_Name} DESTINATION ${DAEMON_LIB_PATH})

## service
file(GLOB OSD_SERVICE_FILES "files/*.service")
install(FILES ${OSD_SERVICE_FILES} DESTINATION share/dbus-1/services)
