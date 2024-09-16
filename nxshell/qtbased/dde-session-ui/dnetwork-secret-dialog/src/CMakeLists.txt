# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Dnetwork_Secret_Dialog_Name dnetwork-secret-dialog)

add_executable(${Dnetwork_Secret_Dialog_Name}
    ${Dnetwork_Secret_Dialog_SRCS}
    main.cpp
    )

target_include_directories(${Dnetwork_Secret_Dialog_Name} PUBLIC
    ${Dnetwork_Secret_Dialog_Includes}
    )

target_link_libraries(${Dnetwork_Secret_Dialog_Name} PRIVATE
    ${Dnetwork_Secret_Dialog_Libraries}
    )

## bin
install(TARGETS ${Dnetwork_Secret_Dialog_Name} DESTINATION ${DAEMON_LIB_PATH})
