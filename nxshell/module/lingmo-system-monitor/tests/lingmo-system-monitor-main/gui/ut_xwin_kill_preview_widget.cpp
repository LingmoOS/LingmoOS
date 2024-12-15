// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "xwin_kill_preview_widget.h"
#include "wm/wm_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QMouseEvent>
using namespace core::wm;
/***************************************STUB begin*********************************************/
ACCESS_PRIVATE_FUN(WMInfo, void(void), buildWindowTreeSchema);
ACCESS_PRIVATE_FUN(WMInfo, void(void), findDockWindows);

void stub_XWinKillPreviewWidget_WMInfo_buildWindowTreeSchema()
{
    return;
}

void stub_XWinKillPreviewWidget_WMInfo_findDockWindows()
{
    return;
}

std::list<WMWindowArea> stub_XWinKillPreviewWidget_WMInfo_selectWindows()
{
    wm_window_area_t area;
    area.pid = 100000;
    area.wid = 111111;
    area.rect = QRect(0, 0, 100, 100);
    std::list<WMWindowArea> list;

    return list;
}

void stub_XWinKillPreviewWidget_show()
{
    return;
}
/***************************************STUB end**********************************************/



class UT_XWinKillPreviewWidget : public ::testing::Test
{
public:
    UT_XWinKillPreviewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        auto wminfo_buildWindowTreeSchema = get_private_fun::WMInfobuildWindowTreeSchema();
        auto wminfo_findDockWindows = get_private_fun::WMInfofindDockWindows();
        Stub stub;
        stub.set(wminfo_buildWindowTreeSchema, stub_XWinKillPreviewWidget_WMInfo_buildWindowTreeSchema);
        stub.set(wminfo_findDockWindows, stub_XWinKillPreviewWidget_WMInfo_findDockWindows);
        stub.set(ADDR(XWinKillPreviewWidget, show), stub_XWinKillPreviewWidget_show);
        static QWidget wid;
        m_tester = new XWinKillPreviewWidget(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    XWinKillPreviewWidget *m_tester;
};

TEST_F(UT_XWinKillPreviewWidget, initTest)
{

}

TEST_F(UT_XWinKillPreviewWidget, test_mousePressEvent_01)
{
    Stub stub;
    stub.set(ADDR(WMInfo, selectWindow), stub_XWinKillPreviewWidget_WMInfo_selectWindows);
    static QMouseEvent me(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(&me);
}

TEST_F(UT_XWinKillPreviewWidget, test_mouseMoveEvent_01)
{
    Stub stub;
    stub.set(ADDR(WMInfo, selectWindow), stub_XWinKillPreviewWidget_WMInfo_selectWindows);
    static QMouseEvent me(QEvent::MouseMove, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mouseMoveEvent(&me);
}

TEST_F(UT_XWinKillPreviewWidget, test_keyPressEvent_01)
{
    static QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_tester->keyPressEvent(&keyEvent);
}

TEST_F(UT_XWinKillPreviewWidget, test_initUI_01)
{
    m_tester->initUI();
}

TEST_F(UT_XWinKillPreviewWidget, test_initConnections_01)
{
    m_tester->initConnections();
}
