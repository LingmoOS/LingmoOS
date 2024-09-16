# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Dmemory_Warning_Dialog_Name dmemory-warning-dialog)

add_executable(${Dmemory_Warning_Dialog_Name}
    ${Dmemory_Warning_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Dmemory_Warning_Dialog_Name} PUBLIC
    ${Dmemory_Warning_Dialog_Includes}
    )

target_link_libraries(${Dmemory_Warning_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Dmemory_Warning_Dialog_Libraries}
    )

## bin
install(TARGETS ${Dmemory_Warning_Dialog_Name} DESTINATION bin)

## service
install(FILES org.deepin.dde.MemoryWarningDialog1.service DESTINATION share/dbus-1/services)
