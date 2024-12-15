// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>

#include "../../src/widgets/filter.h"

using namespace testing;
class HoverShadowFilterTest:public testing::Test{

public:
    HoverShadowFilter *m_hoverShadowFilter;
    virtual void SetUp() override{
        m_hoverShadowFilter = new HoverShadowFilter();
    }

    virtual void TearDown() override{
        if(nullptr != m_hoverShadowFilter)
            delete m_hoverShadowFilter;
    }
};

TEST_F(HoverShadowFilterTest, eventFilter)
{
    QWidget *widget = new QWidget();
    QEvent *enterEvent = new QEvent(QEvent::Enter);
    m_hoverShadowFilter->eventFilter(widget,enterEvent);
    QEvent *enterLeave = new QEvent(QEvent::Leave);
    m_hoverShadowFilter->eventFilter(widget,enterLeave);
    QEvent *enterMove = new QEvent(QEvent::Move);
    m_hoverShadowFilter->eventFilter(widget,enterMove);

    delete widget;
    delete enterEvent;
    delete enterLeave;
    delete enterMove;
}





