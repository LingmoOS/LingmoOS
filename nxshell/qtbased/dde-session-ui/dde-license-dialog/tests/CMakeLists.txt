# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_License_Dialog_Name ut-dde-license-dialog)

file(GLOB_RECURSE UT_License_Dialog_SRCS
    "*.cpp"
    )

add_executable(${UT_License_Dialog_Name}
    ${Common_SRCS}
    ${License_Dialog_SRCS}
    ${UT_License_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_License_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_License_Dialog_Name} PUBLIC
    ${License_Dialog_Includes}
    ../src/
    )

target_link_libraries(${UT_License_Dialog_Name} PRIVATE
    ${License_Dialog_Libraries}
    ${Test_Libraries}
    )
