// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "xwin_kill_preview_background_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <DStyle>


/***************************************STUB begin*********************************************/
Dtk::Widget::DStyle* stub_paintEvent_dstyle()
{
    static Dtk::Widget::DStyle style;
    return  &style;
}
/***************************************STUB end**********************************************/

class UT_XWinKillPreviewBackgroundWidget : public ::testing::Test
{
public:
    UT_XWinKillPreviewBackgroundWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        static QPixmap pixmap;
        m_tester = new XWinKillPreviewBackgroundWidget(pixmap, &wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    XWinKillPreviewBackgroundWidget *m_tester;
};

TEST_F(UT_XWinKillPreviewBackgroundWidget, initTest)
{

}

TEST_F(UT_XWinKillPreviewBackgroundWidget, test_updateSelection_01)
{
    m_tester->updateSelection(QRegion());

    EXPECT_EQ(m_tester->m_selRegion, QRegion());
}

TEST_F(UT_XWinKillPreviewBackgroundWidget, test_clearSelection_01)
{
    m_tester->clearSelection();

    EXPECT_EQ(m_tester->m_selRegion, QRegion());
}

TEST_F(UT_XWinKillPreviewBackgroundWidget, test_paintEvent_01)
{
    Stub stub;
    stub.set(ADDR(QWidget, style), stub_paintEvent_dstyle);
    EXPECT_TRUE(!m_tester->grab().isNull());
}

TEST_F(UT_XWinKillPreviewBackgroundWidget, test_initUI_01)
{
    m_tester->initUI();
}

TEST_F(UT_XWinKillPreviewBackgroundWidget, test_initConnection_01)
{
    m_tester->initConnection();
}
