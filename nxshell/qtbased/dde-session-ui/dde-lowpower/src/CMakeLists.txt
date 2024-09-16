# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Lowpower_Name dde-lowpower)

add_executable(${Lowpower_Name}
    ${Lowpower_SRCS}
    main.cpp
)

target_include_directories(${Lowpower_Name} PUBLIC
    ${Lowpower_Includes}
)

target_link_libraries(${Lowpower_Name} PRIVATE
    ${Lowpower_Libraries}
)

## bin
install(TARGETS ${Lowpower_Name} DESTINATION ${DAEMON_LIB_PATH})
