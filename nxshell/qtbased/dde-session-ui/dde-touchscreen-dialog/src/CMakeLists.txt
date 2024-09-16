# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Touchscreen_Dialog_Name dde-touchscreen-dialog)

add_executable(${Touchscreen_Dialog_Name}
    ${Touchscreen_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Touchscreen_Dialog_Name} PUBLIC
    ${Touchscreen_Dialog_Includes}
    )

target_link_libraries(${Touchscreen_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Touchscreen_Dialog_Libraries}
    )

## bin
install(TARGETS ${Touchscreen_Dialog_Name} DESTINATION ${DAEMON_LIB_PATH})
