// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/filter.h"

using namespace testing;
class HoverFilterTest:public testing::Test{

public:
    HoverFilter *m_hoverFilter;
    virtual void SetUp() override{
        m_hoverFilter = new HoverFilter();
    }

    virtual void TearDown() override{
        if(nullptr != m_hoverFilter)
            delete m_hoverFilter;
    }
};

TEST_F(HoverFilterTest, eventFilter)
{
    QWidget *widget = new QWidget();
    QEvent *enterEvent = new QEvent(QEvent::Enter);
    m_hoverFilter->eventFilter(widget,enterEvent);
    QEvent *enterLeave = new QEvent(QEvent::Leave);
    m_hoverFilter->eventFilter(widget,enterLeave);
    QEvent *enterMove = new QEvent(QEvent::Move);
    m_hoverFilter->eventFilter(widget,enterMove);

    delete widget;
    delete enterEvent;
    delete enterLeave;
    delete enterMove;
}


