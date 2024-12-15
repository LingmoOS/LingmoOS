// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QTimer>

#define private public
#define protected public
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/shottoolwidget.h"

using namespace testing;
bool isChecked_true_stub()
{
    return true;
}
class ShotToolWidgetTest:public testing::Test{

public:
    Stub stub;
    ShotToolWidget *m_shotToolWidget;
    QString m_tips = QString("tips");
    virtual void SetUp() override{
        m_shotToolWidget = new ShotToolWidget();
//        m_shotToolWidget->initWidget();
//        m_shotToolWidget->initRectLabel();
//        m_shotToolWidget->initCircLabel();
//        m_shotToolWidget->initLineLabel();
//        m_shotToolWidget->initPenLabel();
//        m_shotToolWidget->initTextLabel();
    }

    virtual void TearDown() override{
        if(nullptr != m_shotToolWidget)
            delete m_shotToolWidget;
    }
};

TEST_F(ShotToolWidgetTest, initWidget)
{
    m_shotToolWidget->initWidget();
}

TEST_F(ShotToolWidgetTest, initRectLabel)
{
    m_shotToolWidget->initWidget();
    stub.set(ADDR(QAbstractButton,isChecked),isChecked_true_stub);
    m_shotToolWidget->initRectLabel();
    stub.reset(ADDR(QAbstractButton,isChecked));
    m_shotToolWidget->initRectLabel();
    emit m_shotToolWidget->m_blurRectButton->clicked(true);
    emit m_shotToolWidget->m_mosaicRectButton->clicked(true);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ShotToolWidgetTest, initCircLabel)
{
    m_shotToolWidget->initWidget();
    stub.set(ADDR(QAbstractButton,isChecked),isChecked_true_stub);
    m_shotToolWidget->initCircLabel();
    stub.reset(ADDR(QAbstractButton,isChecked));
    m_shotToolWidget->initCircLabel();

    emit m_shotToolWidget->m_blurCircButton->clicked(true);
    emit m_shotToolWidget->m_mosaicCircButton->clicked(true);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ShotToolWidgetTest, initLineLabel)
{
    m_shotToolWidget->initLineLabel();
}

TEST_F(ShotToolWidgetTest, initPenLabel)
{
    m_shotToolWidget->initPenLabel();
}

TEST_F(ShotToolWidgetTest, initTextLabel)
{
    m_shotToolWidget->initTextLabel();
}

TEST_F(ShotToolWidgetTest, colorChecked)
{
//    m_shotToolWidget->m_blurRectButton = new ToolButton();
//    m_shotToolWidget->m_mosaicRectButton = new ToolButton();
//    m_shotToolWidget->m_blurCircButton = new ToolButton();
//    m_shotToolWidget->m_mosaicCircButton = new ToolButton();
//    m_shotToolWidget->m_blurRectButton->setChecked(false);
//    m_shotToolWidget->m_mosaicRectButton->setChecked(false);
//    m_shotToolWidget->m_blurCircButton->setChecked(false);
//    m_shotToolWidget->m_mosaicCircButton->setChecked(false);
    m_shotToolWidget->colorChecked(QString("rectangle"));
}



