# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(License_Dialog_Name dde-license-dialog)

add_executable(${License_Dialog_Name}
    ${Common_SRCS}
    ${License_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${License_Dialog_Name} PUBLIC
    ${License_Dialog_Includes}
    )

target_link_libraries(${License_Dialog_Name} PRIVATE
    ${License_Dialog_Libraries}
    )

## bin
install(TARGETS ${License_Dialog_Name} DESTINATION ${CMAKE_INSTALL_BINDIR})
