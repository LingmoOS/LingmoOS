# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(UT_OSD_Name ut-dde-osd)

add_executable(${UT_OSD_Name}
    mocknotifysetting.cpp
    mocknotifysetting.h
    mockpersistence.cpp
    mockpersistence.h
    ut_audioprovider.cpp
    ut_brightnessprovider.cpp
    ut_common.cpp
    ut_container.cpp
    ut_dde-osd_main.cpp
    ut_delegate.cpp
    ut_displaymodeprovider.cpp
    ut_kblayoutprovider.cpp
    ut_listview.cpp
    ut_manager.cpp
    ut_osdprovider.cpp

    notification/ut_actionbutton.cpp
    notification/ut_appbody.cpp
    notification/ut_appbodylabel.cpp
    notification/ut_appicon.cpp
    notification/ut_bubble.cpp
    notification/ut_bubblemanager.cpp
    notification/ut_bubbletool.cpp
    notification/ut_button.cpp
    notification/ut_dockrect.cpp
    notification/ut_iconbutton.cpp
    notification/ut_notificationentity.cpp

    notification-center/ut_bubbleitem.cpp
    notification-center/ut_bubbletitlewidget.cpp
    notification-center/ut_notifycenterwidget.cpp
    notification-center/ut_notifyListview.cpp
    notification-center/ut_notifywidget.cpp
    notification-center/ut_overlapwidget.cpp
)

# 用于测试覆盖率的编译条件
target_compile_options(${UT_OSD_Name} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_OSD_Name} PUBLIC
    Qt5::Test
    ../src/
    ../src/notification/
    ../src/notification-center/
    )

target_link_libraries(${UT_OSD_Name} PRIVATE
    dde-osd-shared
    session-ui-dbus-shared
    Qt5::Test
    ${Test_Libraries}
    )
