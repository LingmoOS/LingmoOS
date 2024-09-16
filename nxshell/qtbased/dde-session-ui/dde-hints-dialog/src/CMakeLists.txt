# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Hints_Dialog_Name dde-hints-dialog)

add_executable(${Hints_Dialog_Name}
    ${Hints_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Hints_Dialog_Name} PUBLIC
    ${Hints_Dialog_Includes}
    )

target_link_libraries(${Hints_Dialog_Name} PRIVATE
    ${Hints_Dialog_Libraries}
    )

## bin
install(TARGETS ${Hints_Dialog_Name} DESTINATION ${CMAKE_INSTALL_BINDIR})
