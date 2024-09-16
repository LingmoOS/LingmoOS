// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "base/base_item_delegate.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QTableView>
#include <DStyle>
DWIDGET_USE_NAMESPACE;
/***************************************STUB begin*********************************************/

bool stub_BaseItemDelegate_paint_isVaild()
{
    return true;
}

int stub_helpEvent_column()
{
    return 0;
}
/***************************************STUB end**********************************************/


class UT_BaseItemDelegate : public ::testing::Test
{
public:
    UT_BaseItemDelegate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget obj;
        m_tester = new BaseItemDelegate(&obj);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BaseItemDelegate *m_tester;
};

TEST_F(UT_BaseItemDelegate, initTest)
{

}

TEST_F(UT_BaseItemDelegate, test_paint_01)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->paint(&painter, option, index);
}


TEST_F(UT_BaseItemDelegate, test_paint_02)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_Selected | DStyle::State_Sunken;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_03)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_Selected | DStyle::State_MouseOver;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_04)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_05)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;
    option.viewItemPosition = QStyleOptionViewItem::Beginning;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_06)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;
    option.viewItemPosition = QStyleOptionViewItem::Middle;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_07)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;
    option.viewItemPosition = QStyleOptionViewItem::End;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_08)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;
    option.viewItemPosition = QStyleOptionViewItem::OnlyOne;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseItemDelegate, test_paint_09)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    option.state = DStyle::State_Enabled | DStyle::State_MouseOver;
    option.viewItemPosition = QStyleOptionViewItem::OnlyOne;
    option.features = QStyleOptionViewItem::HasDecoration;

    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_BaseItemDelegate_paint_isVaild);
    m_tester->paint(&painter, option, index);
}


TEST_F(UT_BaseItemDelegate, test_createEditor_01)
{
    static QWidget wid;
    static QStyleOptionViewItem option;
    static QModelIndex index;
    EXPECT_EQ(m_tester->createEditor(&wid, option, index), nullptr);
}

TEST_F(UT_BaseItemDelegate, test_sizeHint_01)
{
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->sizeHint(option, index);
}

TEST_F(UT_BaseItemDelegate, test_helpEvent_01)
{
    static QHelpEvent he(QEvent::ToolTip, QPoint(0, 0), QPoint(0, 0));
    static QWidget parent;
    static QTableView view(&parent);
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->helpEvent(&he, &view, option, index);
}

TEST_F(UT_BaseItemDelegate, test_helpEvent_02)
{
    static QHelpEvent he(QEvent::ToolTip, QPoint(0, 0), QPoint(0, 0));
    static QWidget parent;
    static QTableView view(&parent);
    static QStyleOptionViewItem option;
    option.features =  QStyleOptionViewItem::HasDecoration;
    Stub stub;
    stub.set(ADDR(QModelIndex, column), stub_helpEvent_column);
    static QModelIndex index;
    m_tester->helpEvent(&he, &view, option, index);
}

TEST_F(UT_BaseItemDelegate, test_initStyleOption_01)
{
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->initStyleOption(&option, index);
}
