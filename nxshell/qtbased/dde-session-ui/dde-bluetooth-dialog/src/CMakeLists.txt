# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Bluetooth_Dialog_Name dde-bluetooth-dialog)

add_executable(${Bluetooth_Dialog_Name}
    ${Common_SRCS}
    ${Bluetooth_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Bluetooth_Dialog_Name} PUBLIC
    ${Bluetooth_Dialog_Includes}
    )

target_link_libraries(${Bluetooth_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Bluetooth_Dialog_Libraries}
    )

## bin
install(TARGETS ${Bluetooth_Dialog_Name} DESTINATION ${DAEMON_LIB_PATH})
