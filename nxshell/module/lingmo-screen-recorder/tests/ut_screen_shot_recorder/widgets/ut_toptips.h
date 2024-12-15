// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSysInfo>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/widgets/toptips.h"

using namespace testing;
//ACCESS_PRIVATE_FUN(TopTips, void(QMouseEvent *ev), mousePressEvent);
bool startsWith_stub(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    Q_UNUSED(s);
    Q_UNUSED(cs);
    return true;
}

class TopTipsTest:public testing::Test{

public:
    Stub stub;
    TopTips *m_topTips;
    virtual void SetUp() override{
        m_topTips = new TopTips();
        stub.set((bool(QString::*)(const QString &s, Qt::CaseSensitivity cs) const)ADDR(QString,startsWith),startsWith_stub);
    }

    virtual void TearDown() override{
        if(nullptr != m_topTips)
            delete m_topTips;
        stub.reset((bool(QString::*)(const QString &s, Qt::CaseSensitivity cs) const)ADDR(QString,startsWith));
    }
};

TEST_F(TopTipsTest, setRecorderTipsInfo)
{
    m_topTips->setRecorderTipsInfo(false);
}

TEST_F(TopTipsTest, setContent)
{
    m_topTips->setContent(QSize(1920,1040));
    m_topTips->setRecorderTipsInfo(true);
    m_topTips->setContent(QSize(1922,1082));
}

TEST_F(TopTipsTest, updateTips)
{
    m_topTips->updateTips(QPoint(0,0),QSize(1920,1040));
    m_topTips->updateTips(QPoint(200,200),QSize(100,100));
}
/*
TEST_F(TopTipsTest, mousePressEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::TopTipsmousePressEvent(*m_topTips,ev);
}
*/
