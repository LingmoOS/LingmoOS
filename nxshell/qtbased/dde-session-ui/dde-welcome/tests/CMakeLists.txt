# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_Welcome_Name ut-dde-welcome)

add_executable(${UT_Welcome_Name}
    "main.cpp"
    "ut_mainwidget.cpp"
    "ut_updatecontent.cpp"
    "global_util/ut_multiscreenmanager.cpp"
    "global_util/ut_publicfunc.cpp"
    "global_util/ut_utilupdateui.cpp"
    "widgets/ut_fullscreenbackground.cpp"
    "widgets/ut_propertygroup.cpp"
)

# 用于测试覆盖率的编译条件
target_compile_options(${UT_Welcome_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_Welcome_Name} PUBLIC
    ../src/
)

target_link_libraries(${UT_Welcome_Name} PRIVATE
    dde-welcome-shared
    session-ui-widgets-shared
    session-ui-util-shared
    session-ui-dbus-shared
    ${Test_Libraries}
    Qt5::Test
)
