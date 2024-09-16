// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RichTextDelegate.h"
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "PageBoardInfo.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

#include <DStyle>
#include <DApplication>

#include <gtest/gtest.h>

DStyle *rc_style;
class UT_RichTextDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        widget = new PageTableWidget;
        m_rtDelegate = new RichTextDelegate(widget);
        rc_style = new DStyle;
    }
    void TearDown()
    {
        delete m_rtDelegate;
        delete widget;
        delete rc_style;
    }
    RichTextDelegate *m_rtDelegate;
    PageTableWidget *widget;
};

bool ut_richTextDelegate_isValid()
{
    return true;
}

QRect ut_richTextDelegate_subElementRect()
{
    return QRect(10, 10, 10, 10);
}

DStyle *ut_richtextdelegate_style()
{
    return rc_style;
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_paint_001)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);

    PageBoardInfo *par = new PageBoardInfo;
    widget->setParent(par);

    widget->setColumnAndRow(3);
    widget->setItemDelegateForRow(0, m_rtDelegate);
    QTableWidgetItem *itemFirst = new QTableWidgetItem("pairs[i - lst.size()].first");
    widget->setItem(0, 0, itemFirst);
    QTableWidgetItem *itemSecond = new QTableWidgetItem("pairs[i - lst.size()].second");
    widget->setItem(0, 1, itemSecond);

    QModelIndex index = widget->mp_Table->model()->index(0, 1);

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), ut_richTextDelegate_isValid);
    stub.set((QRect(DStyle::*)(DStyle::SubElement, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, subElementRect), ut_richTextDelegate_subElementRect);
    stub.set(ADDR(DApplication, style), ut_richtextdelegate_style);

    m_rtDelegate->paint(&painter, option, index);
    EXPECT_FALSE(widget->grab().isNull());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_createEditor)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    EXPECT_FALSE(m_rtDelegate->createEditor(nullptr, option, index));
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_sizeHint)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    QSize size = m_rtDelegate->sizeHint(option, index);
    EXPECT_EQ(150, size.width());
    EXPECT_EQ(40, size.height());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_addRow)
{
    QDomDocument doc;
    QDomElement table = doc.createElement("table");
    table.setAttribute("style", "border-collapse: collapse;border-spacing: 0;");
    QPair<QString, QString> pair = QPair<QString, QString>("first", "second1  /  \t\tsecond2  /  \t\tsecond3");
    m_rtDelegate->addRow(doc, table, pair,120);
    EXPECT_FALSE(doc.isNull());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_getDocFromLst)
{
    QDomDocument doc;
    doc.setNodeValue("/");
    m_rtDelegate->getDocFromLst(doc, QStringList() << "first:second"
                                << "first");
    EXPECT_FALSE(doc.isNull());
}
