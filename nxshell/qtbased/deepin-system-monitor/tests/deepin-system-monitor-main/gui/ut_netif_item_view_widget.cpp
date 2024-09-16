// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "netif_item_view_widget.h"
#include "system/netif.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>
#include <QMouseEvent>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_NetifItemViewWidget : public ::testing::Test
{
public:
    UT_NetifItemViewWidget() : m_tester(nullptr), m_netInfo(nullptr) {}

public:
    virtual void SetUp()
    {
        m_netInfo = new core::system::NetifInfo;
        static QWidget parent;
        m_tester = new NetifItemViewWidget(&parent);
    }

    virtual void TearDown()
    {
        if (m_netInfo) {
            delete m_netInfo;
            m_netInfo = nullptr;
        }
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifItemViewWidget *m_tester;

    core::system::NetifInfo *m_netInfo;
};

TEST_F(UT_NetifItemViewWidget, initTest)
{

}

TEST_F(UT_NetifItemViewWidget, test_updateActiveStatus_01)
{
    m_tester->updateActiveStatus(true);

    EXPECT_TRUE(m_tester->m_isActive);
}

TEST_F(UT_NetifItemViewWidget, test_setMode_01)
{
    m_tester->setMode(0);

    EXPECT_EQ(m_tester->m_mode, 0);
}

TEST_F(UT_NetifItemViewWidget, test_paintEvent_01)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

TEST_F(UT_NetifItemViewWidget, test_resizeEvent_01)
{
    QResizeEvent ev(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&ev);
}

TEST_F(UT_NetifItemViewWidget, test_mousePressEvent_01)
{
    static QMouseEvent ev(QEvent::MouseButtonPress, QPointF(1.0, 1.0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(&ev);
}

TEST_F(UT_NetifItemViewWidget, test_fontChanged_01)
{
    QFont font;
    font.setItalic(true);
    m_tester->fontChanged(font);

    EXPECT_TRUE(m_tester->m_font.italic());
}

TEST_F(UT_NetifItemViewWidget, test_updateData_01)
{

    std::shared_ptr<class core::system::NetifInfo> netifInfo = std::make_shared<class core::system::NetifInfo>();

    m_tester->updateData(netifInfo);
}

TEST_F(UT_NetifItemViewWidget, test_updateWidgetGeometry_01)
{
    m_tester->updateWidgetGeometry();
}
