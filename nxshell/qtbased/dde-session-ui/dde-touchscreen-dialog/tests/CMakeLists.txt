# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Touchscreen_Dialog_Name ut-dde-touchscreen-dialog)

file(GLOB_RECURSE UT_Touchscreen_Dialog_SRCS
    "*.h"
    "*.cpp"
    )
add_executable(${UT_Touchscreen_Dialog_Name}
    ${Touchscreen_Dialog_SRCS}
    ${UT_Touchscreen_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Touchscreen_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Touchscreen_Dialog_Name} PUBLIC
    ${Touchscreen_Dialog_Includes}
    dde-touchscreen-dialog/
    ../src/
    )

target_link_libraries(${UT_Touchscreen_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Touchscreen_Dialog_Libraries}
    ${Test_Libraries}
    )
