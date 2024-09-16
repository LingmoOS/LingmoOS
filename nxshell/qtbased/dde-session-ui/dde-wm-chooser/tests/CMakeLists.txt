# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Wm_Chooser_Name ut-dde-wm-chooser)

file(GLOB_RECURSE UT_Wm_Chooser_SRCS
    "*.h"
    "*.cpp"
    )

add_executable(${UT_Wm_Chooser_Name}
    ${Wm_Chooser_SRCS}
    ${UT_Wm_Chooser_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Wm_Chooser_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Wm_Chooser_Name} PUBLIC
    ${Wm_Chooser_Includes}
    Qt5::Test
    ../src/
    )

target_link_libraries(${UT_Wm_Chooser_Name} PRIVATE
    session-ui-widgets-shared
    session-ui-util-shared
    session-ui-dbus-shared
    ${Wm_Chooser_Libraries}
    ${Test_Libraries}
    Qt5::Test
    )
