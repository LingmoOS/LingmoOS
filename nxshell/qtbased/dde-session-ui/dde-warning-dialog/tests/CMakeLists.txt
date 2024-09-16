# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Warning_Dialog_Name ut-dde-warning-dialog)

file(GLOB_RECURSE UT_Warning_Dialog_SRCS
    "*.h"
    "*.cpp"
    )
add_executable(${UT_Warning_Dialog_Name}
    ${Warning_Dialog_SRCS}
    ${UT_Warning_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Warning_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Warning_Dialog_Name} PUBLIC
    ${Warning_Dialog_Includes}
    ../src/
    )

target_link_libraries(${UT_Warning_Dialog_Name} PRIVATE
    ${Warning_Dialog_Libraries}
    ${Test_Libraries}
    )
