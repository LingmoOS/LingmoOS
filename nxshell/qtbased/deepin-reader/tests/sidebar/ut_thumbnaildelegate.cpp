// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ThumbnailDelegate.h"
#include "DocSheet.h"
#include "SideBarImageListview.h"
#include "SideBarImageViewModel.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QListView>
#include <QPainter>

class UT_ThumbnailDelegate : public ::testing::Test
{
public:
    UT_ThumbnailDelegate() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_pView = new SideBarImageListView(m_sheet);
        m_tester = new ThumbnailDelegate(m_pView);
        m_pView->setItemDelegate(m_tester);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_pView;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    ThumbnailDelegate *m_tester = nullptr;
    SideBarImageListView *m_pView = nullptr;
};

TEST_F(UT_ThumbnailDelegate, initTest)
{

}

TEST_F(UT_ThumbnailDelegate, UT_ThumbnailDelegate_paint)
{
    m_pView->getImageModel()->insertPageIndex(1);
    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    m_tester->paint(painter, option, m_pView->getImageModel()->index(0, 0));
    EXPECT_TRUE(m_tester->m_parent == m_pView);
    delete painter;
}

TEST_F(UT_ThumbnailDelegate, UT_ThumbnailDelegate_drawBookMark)
{
    QPainter *painter = new QPainter();
    m_tester->drawBookMark(painter, m_pView->geometry(), true);
    EXPECT_TRUE(m_pView != nullptr);
    delete painter;
}
