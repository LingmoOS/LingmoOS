# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Wm_Chooser_Name dde-wm-chooser)

add_executable(${Wm_Chooser_Name}
    ${Wm_Chooser_SRCS}
    main.cpp
    )

target_include_directories(${Wm_Chooser_Name} PUBLIC
    ${Wm_Chooser_Includes}
    )

target_link_libraries(${Wm_Chooser_Name} PRIVATE
    session-ui-widgets-shared
    session-ui-util-shared
    session-ui-dbus-shared
    ${Wm_Chooser_Libraries}
    )

## bin
install(TARGETS ${Wm_Chooser_Name} DESTINATION ${CMAKE_INSTALL_BINDIR})
