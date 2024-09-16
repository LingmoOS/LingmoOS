# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Suspend_Dialog_Name ut-dde-suspend-dialog)

file(GLOB_RECURSE UT_Suspend_Dialog_SRCS
    "*.h"
    "*.cpp"
    )
add_executable(${UT_Suspend_Dialog_Name}
    ${Suspend_Dialog_SRCS}
    ${UT_Suspend_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Suspend_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Suspend_Dialog_Name} PUBLIC
    ${Suspend_Dialog_Includes}
    Qt5::Test
    dde-suspend-dialog/
    ../src/
    )

target_link_libraries(${UT_Suspend_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Suspend_Dialog_Libraries}
    ${Test_Libraries}
    Qt5::Test
    )
