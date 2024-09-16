// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageOverview.h"
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "LongTextLabel.h"
#include "DeviceInput.h"
#include "ut_Head.h"
#include "stub.h"

#include <DSysInfo>
#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMenu>

#include <gtest/gtest.h>

class UT_PageOverview : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageOverview = new PageOverview;
    }
    void TearDown()
    {
        delete m_pageOverview;
    }
    PageOverview *m_pageOverview = nullptr;
};

TEST_F(UT_PageOverview, UT_PageOverview_updateInfo2)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_pageOverview->updateInfo(mapinfo);
    m_pageOverview->clearWidgets();
    m_pageOverview->setLabel("UnionTech OS Desktop", "UOS");
    EXPECT_STREQ("UnionTech OS Desktop", m_pageOverview->mp_DeviceLabel->text().toStdString().c_str());
}

TEST_F(UT_PageOverview, UT_PageOverview_slotActionRefresh)
{
    QString conStr = m_pageOverview->mp_Overview->toString();
    m_pageOverview->slotActionCopy();
    QClipboard *clipboard = DApplication::clipboard();
    QString clipStr = clipboard->text();
    EXPECT_STREQ(conStr.toStdString().c_str(), clipStr.toStdString().c_str());
    EXPECT_TRUE(m_pageOverview->isOverview());
}

void ut_overview_exec()
{
    return;
}

TEST_F(UT_PageOverview, UT_PageOverview_slotShowMenu)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_overview_exec);

    m_pageOverview->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(m_pageOverview->mp_Menu->actions().size(), 3);
}
