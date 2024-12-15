// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "base/base_detail_item_delegate.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>
#include <QHelpEvent>
#include <QTableView>
#include <QPainter>

/***************************************STUB begin*********************************************/
QRect stub_paint_rect()
{
    return QRect(0, 0, 100, 100);
}

bool stub_paint_isVaild()
{
    return true;
}

/***************************************STUB end**********************************************/


class UT_BaseDetailItemDelegate : public ::testing::Test
{
public:
    UT_BaseDetailItemDelegate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QObject obj;
        m_tester = new BaseDetailItemDelegate(&obj);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BaseDetailItemDelegate *m_tester;
};

TEST_F(UT_BaseDetailItemDelegate, initTest)
{

}

TEST_F(UT_BaseDetailItemDelegate, test_helpEvent_01)
{
    static QHelpEvent he(QEvent::ToolTip, QPoint(0, 0), QPoint(0, 0));
    static QWidget parent;
    static QTableView view(&parent);
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->helpEvent(&he, &view, option, index);
}

TEST_F(UT_BaseDetailItemDelegate, test_paint_01)
{
    static QPixmap pix(100, 100);
    static QPainter painter(&pix);
    static QStyleOptionViewItem option;
    static QWidget optionWid;
    optionWid.setFixedSize(100, 100);
    option.widget = &optionWid;
    static QModelIndex index;

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_paint_isVaild);
    m_tester->paint(&painter, option, index);
}

TEST_F(UT_BaseDetailItemDelegate, test_sizeHint_01)
{
    static QStyleOptionViewItem option;
    static QModelIndex index;
    m_tester->sizeHint(option, index);
}

