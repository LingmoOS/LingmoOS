# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Warning_Dialog_Name dde-warning-dialog)

add_executable(${Warning_Dialog_Name}
    ${Warning_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Warning_Dialog_Name} PUBLIC
    ${Warning_Dialog_Includes}
    )

target_link_libraries(${Warning_Dialog_Name} PRIVATE
    ${Warning_Dialog_Libraries}
    )

## bin
install(TARGETS ${Warning_Dialog_Name} DESTINATION ${DAEMON_LIB_PATH})

## service
install(FILES org.deepin.dde.WarningDialog1.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
