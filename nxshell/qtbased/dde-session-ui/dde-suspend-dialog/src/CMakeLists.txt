# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Suspend_Dialog_Name dde-suspend-dialog)

add_executable(${Suspend_Dialog_Name}
    ${Suspend_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Suspend_Dialog_Name} PUBLIC
    ${Suspend_Dialog_Includes}
    )

target_link_libraries(${Suspend_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Suspend_Dialog_Libraries}
    )

## bin
install(TARGETS ${Suspend_Dialog_Name} DESTINATION ${DAEMON_LIB_PATH})

## icons
install(FILES data/computer.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/devices)
