// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "detail_view_stacked_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QMenu>
#include <QAction>

/***************************************STUB begin*********************************************/

QAction* stub_onShowPerformMenu_exec(void* obj, const QPoint &p, QAction *action = nullptr)
{
    Q_UNUSED(p);
    Q_UNUSED(obj);
    Q_UNUSED(action);

    return  nullptr;
}

/***************************************STUB end**********************************************/

class UT_DetailViewStackedWidget : public ::testing::Test
{
public:
    UT_DetailViewStackedWidget() : m_tester(nullptr),
    m_widget(nullptr),
    m_cpuWidget(nullptr),
    m_netWidget(nullptr),
    m_memWidget(nullptr),
    m_diskWidget(nullptr){}

public:
    virtual void SetUp()
    {
        m_tester = new DetailViewStackedWidget(nullptr);
        m_widget = new QWidget;
        m_cpuWidget = new QWidget;
        m_netWidget = new QWidget;
        m_memWidget = new QWidget;
        m_diskWidget = new QWidget;
    }

    virtual void TearDown()
    {
        m_widget->deleteLater();
        m_cpuWidget->deleteLater();
        m_netWidget->deleteLater();
        m_memWidget->deleteLater();
        m_diskWidget->deleteLater();
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }


protected:
    DetailViewStackedWidget *m_tester;
    QWidget *m_widget;
    QWidget *m_cpuWidget;
    QWidget *m_netWidget;
    QWidget *m_memWidget;
    QWidget *m_diskWidget;
};

TEST_F(UT_DetailViewStackedWidget, initTest)
{

}

TEST_F(UT_DetailViewStackedWidget, test_addProcessWidget_01)
{
    m_tester->addProcessWidget(m_widget);

    EXPECT_NE(m_tester->m_processWidget, nullptr);
}


TEST_F(UT_DetailViewStackedWidget, test_deleteDetailPage_01)
{
    m_tester->deleteDetailPage();

    EXPECT_EQ(m_tester->m_cpudetailWidget, nullptr);
    EXPECT_EQ(m_tester->m_memDetailWidget, nullptr);
    EXPECT_EQ(m_tester->m_netifDetailWidget, nullptr);
    EXPECT_EQ(m_tester->m_blockDevDetailWidget, nullptr);
}

TEST_F(UT_DetailViewStackedWidget, test_onShowPerformMenu_01)
{
    Stub stub;
    stub.set((QAction*(QMenu::*)(const QPoint&, QAction*))ADDR(QMenu, exec), stub_onShowPerformMenu_exec);
    m_tester->onShowPerformMenu(QPoint(1, 1));

    EXPECT_EQ(m_tester->memAct->isCheckable(), true);
}

TEST_F(UT_DetailViewStackedWidget, test_onShowPerformMenu_02)
{
    static DMenu menu;
    m_tester->m_menu = &menu;
    static QAction cpuAct;
    m_tester->cpuAct = &cpuAct;
    Stub stub;
    stub.set((QAction*(QMenu::*)(const QPoint&, QAction*))ADDR(QMenu, exec), stub_onShowPerformMenu_exec);
    m_tester->onShowPerformMenu(QPoint(1, 1));
}

TEST_F(UT_DetailViewStackedWidget, test_onDetailInfoClicked_01)
{
    m_tester->m_lastDteailWidgetName = "MemDetailViewWidget";
    m_tester->onDetailInfoClicked();
    EXPECT_EQ(m_tester->currentWidget(), m_tester->m_memDetailWidget);
}

TEST_F(UT_DetailViewStackedWidget, test_onDetailInfoClicked_02)
{
    m_tester->m_lastDteailWidgetName = "NetifDetailViewWidget";
    m_tester->onDetailInfoClicked();
    EXPECT_EQ(m_tester->currentWidget(), m_tester->m_netifDetailWidget);
}

TEST_F(UT_DetailViewStackedWidget, test_onDetailInfoClicked_03)
{
    m_tester->m_lastDteailWidgetName = "BlockDevDetailViewWidget";
    m_tester->onDetailInfoClicked();
    EXPECT_EQ(m_tester->currentWidget(), m_tester->m_blockDevDetailWidget);
}

TEST_F(UT_DetailViewStackedWidget, test_onDetailInfoClicked_04)
{
    m_tester->m_lastDteailWidgetName = "";
    m_tester->onDetailInfoClicked();
    EXPECT_EQ(m_tester->currentWidget(), m_tester->m_cpudetailWidget);
}

TEST_F(UT_DetailViewStackedWidget, test_onSwitchProcessPage_01)
{
    m_tester->onSwitchProcessPage();
    EXPECT_EQ(m_tester->currentWidget(), m_tester->m_processWidget);
}

TEST_F(UT_DetailViewStackedWidget, test_onSwitchPageFinished_01)
{
    m_tester->onSwitchPageFinished();
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_01)
{
    QString msgCode = "MSG_CPU";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_011)
{
    m_tester->m_cpudetailWidget = m_cpuWidget;
    QString msgCode = "MSG_CPU";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_02)
{
    QString msgCode = "MSG_NET";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_021)
{
    m_tester->m_netifDetailWidget = m_netWidget;
    QString msgCode = "MSG_NET";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_03)
{
    QString msgCode = "MSG_MEM";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_031)
{
    m_tester->m_memDetailWidget = m_memWidget;
    QString msgCode = "MSG_MEM";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_04)
{
    QString msgCode = "MSG_DISK";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}

TEST_F(UT_DetailViewStackedWidget, test_onDbusSendMsgChangeDetailInfoWidget_041)
{
    m_tester->m_blockDevDetailWidget = m_diskWidget;
    QString msgCode = "MSG_DISK";
    m_tester->onDbusSendMsgChangeDetailInfoWidget(msgCode);
}


