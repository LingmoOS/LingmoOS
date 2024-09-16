# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Welcome_Name dde-welcome)

add_executable(${Welcome_Name}
    main.cpp
)

target_link_libraries(${Welcome_Name}
PRIVATE
    dde-welcome-shared
    session-ui-widgets-shared
    session-ui-util-shared
    session-ui-dbus-shared
)

## bin
install(TARGETS ${Welcome_Name} DESTINATION ${DAEMON_LIB_PATH})

## service
install(FILES org.deepin.dde.Welcome1.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
