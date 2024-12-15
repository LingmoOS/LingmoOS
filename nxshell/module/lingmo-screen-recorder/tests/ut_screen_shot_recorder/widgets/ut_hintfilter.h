// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QTimer>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/tooltips.h"
#include "../../src/widgets/filter.h"

using namespace testing;
bool toBool_stub_true()
{
    return true;
}
bool toBool_stub_false()
{
    return false;
}
ACCESS_PRIVATE_FIELD(HintFilter,QScopedPointer<HintFilterPrivate>, d_ptr)
class HintFilterTest:public testing::Test{

public:
    Stub stub;
    HintFilter *m_hintFilter;
    virtual void SetUp() override{
        m_hintFilter = new HintFilter();
    }

    virtual void TearDown() override{
        if(nullptr != m_hintFilter)
            delete m_hintFilter;
    }
};

TEST_F(HintFilterTest, eventFilter)
{        
    QWidget *widget = new QWidget();
    auto hintWidget = new ToolTips("", nullptr);
    widget->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hintWidget));
    widget->installEventFilter(m_hintFilter);
    access_private_field::HintFilterd_ptr(*m_hintFilter)->parentWidget = nullptr;
    access_private_field::HintFilterd_ptr(*m_hintFilter)->showHint(widget);
    access_private_field::HintFilterd_ptr(*m_hintFilter)->hintWidget = nullptr;
    access_private_field::HintFilterd_ptr(*m_hintFilter)->showHint(widget);
    QWidget *w = new QWidget();
    access_private_field::HintFilterd_ptr(*m_hintFilter)->hintWidget = w;
    access_private_field::HintFilterd_ptr(*m_hintFilter)->showHint(widget);
    QEvent *enterEvent = new QEvent(QEvent::Enter);
    m_hintFilter->eventFilter(widget,enterEvent);
    QEvent *enterLeave = new QEvent(QEvent::Leave);
    m_hintFilter->eventFilter(widget,enterLeave);
    access_private_field::HintFilterd_ptr(*m_hintFilter)->hintWidget = nullptr;
    QEvent *enterLeave1 = new QEvent(QEvent::Leave);
    m_hintFilter->eventFilter(widget,enterLeave1);
    QEvent *enterMove = new QEvent(QEvent::Move);
    m_hintFilter->eventFilter(widget,enterMove);
//    QEvent *enterEvent1 = new QEvent(QEvent::Enter);
//    stub.set(ADDR(QVariant,toBool),toBool_stub_true);
//    m_hintFilter->eventFilter(widget,enterEvent1);
//    stub.reset(ADDR(QVariant,toBool));
//    QEvent *enterEvent2 = new QEvent(QEvent::Enter);
//    stub.set(ADDR(QVariant,toBool),toBool_stub_false);
//    m_hintFilter->eventFilter(widget,enterEvent2);
//    stub.reset(ADDR(QVariant,toBool));

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    delete widget;
    delete hintWidget;
    delete w;
    delete enterEvent;
    delete enterLeave;
    delete enterLeave1;
    delete enterMove;
}


