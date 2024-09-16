# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Lowpower_Name ut-dde-lowpower)

file(GLOB_RECURSE UT_Lowpower_SRCS
    "*.h"
    "*.cpp"
    )

add_executable(${UT_Lowpower_Name}
    ${Lowpower_SRCS}
    ${UT_Lowpower_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Lowpower_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Lowpower_Name} PUBLIC
    ${Lowpower_Includes}
    Qt5::Test
    ../src/
    )

target_link_libraries(${UT_Lowpower_Name} PRIVATE
    ${Lowpower_Libraries}
    ${Test_Libraries}
    Qt5::Test
    )
