# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Dnetwork_Secret_Dialog_Name ut-dnetwork-secret-dialog)

file(GLOB_RECURSE UT_Dnetwork_Secret_Dialog_SRCS
    "*.h"
    "*.cpp"
    )

add_executable(${UT_Dnetwork_Secret_Dialog_Name}
    ${Dnetwork_Secret_Dialog_SRCS}
    ${UT_Dnetwork_Secret_Dialog_SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Dnetwork_Secret_Dialog_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Dnetwork_Secret_Dialog_Name} PUBLIC
    ${Dnetwork_Secret_Dialog_Includes}
    ../src/
    )

target_link_libraries(${UT_Dnetwork_Secret_Dialog_Name} PRIVATE
    ${Dnetwork_Secret_Dialog_Libraries}
    ${Test_Libraries}
    )
