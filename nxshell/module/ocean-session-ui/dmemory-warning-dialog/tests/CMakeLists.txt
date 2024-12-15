# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Dmemory_Warning_Dialog_Name ut-dmemory-warning-dialog)

file(GLOB_RECURSE UT_Dmemory_Warning_Dialog_SRCS
    "*.h"
    "*.cpp"
    )
add_executable(${UT_Dmemory_Warning_Dialog_Name}
    ${Dmemory_Warning_Dialog_SRCS}
    ${UT_Dmemory_Warning_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Dmemory_Warning_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Dmemory_Warning_Dialog_Name} PUBLIC
    ${Dmemory_Warning_Dialog_Includes}
    Qt5::Test
    ../src/
    )

target_link_libraries(${UT_Dmemory_Warning_Dialog_Name} PRIVATE
    session-ui-dbus-shared
    ${Dmemory_Warning_Dialog_Libraries}
    ${Test_Libraries}
    Qt5::Test
    )
