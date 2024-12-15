// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QObject>
#include <QScreen>
#include <QDesktopWidget>
#include <QDebug>
#include <QPainter>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/countdown_tooltip.h"
#include "../../src/utils/tempfile.h"


using namespace testing;

class CountdownTooltipTest: public testing::Test
{

public:
    Stub stub;
    CountdownTooltip *m_countTip = new CountdownTooltip;
    virtual void SetUp() override
    {
        std::cout << "start CountdownTooltipTest" << std::endl;

    }

    virtual void TearDown() override
    {
        m_countTip->hide();
        delete m_countTip;
        std::cout << "end CountdownTooltipTest" << std::endl;
    }
public slots:
    void startRecord();
};

TEST_F(CountdownTooltipTest, start)
{
    m_countTip->show();
    m_countTip->start();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
}
void CountdownTooltipTest::startRecord()
{
    qDebug() << "startRecord";
}

QPixmap getFullscreenPixmap_stub()
{

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);

}

ACCESS_PRIVATE_FUN(CountdownTooltip, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FIELD(CountdownTooltip, int, showCountdownCounter);
TEST_F(CountdownTooltipTest, paintEvent)
{
    access_private_field::CountdownTooltipshowCountdownCounter(*m_countTip) = 1;
    stub.set(ADDR(TempFile, getFullscreenPixmap), getFullscreenPixmap_stub);
    QPaintEvent *event;
    //执行函数case
    call_private_fun::CountdownTooltippaintEvent(*m_countTip, event);
    stub.reset(ADDR(TempFile, getFullscreenPixmap));
}

ACCESS_PRIVATE_FUN(CountdownTooltip, QPixmap(), getTooltipBackground);
TEST_F(CountdownTooltipTest, getTooltipBackground)
{
    stub.set(ADDR(TempFile, getFullscreenPixmap), getFullscreenPixmap_stub);
    //执行函数case
    QPixmap res = call_private_fun::CountdownTooltipgetTooltipBackground(*m_countTip);
    stub.reset(ADDR(TempFile, getFullscreenPixmap));
}

ACCESS_PRIVATE_FUN(CountdownTooltip, void(QPainter &, QPixmap &), paintRect);
TEST_F(CountdownTooltipTest, paintRect)
{

    stub.set(ADDR(TempFile, getFullscreenPixmap), getFullscreenPixmap_stub);
    //执行函数case
    QPixmap res = call_private_fun::CountdownTooltipgetTooltipBackground(*m_countTip);
    stub.reset(ADDR(TempFile, getFullscreenPixmap));

    QPainter *painter = new QPainter();
    //执行函数case
    call_private_fun::CountdownTooltippaintRect(*m_countTip, *painter, res);

    delete painter;
}

ACCESS_PRIVATE_FIELD(CountdownTooltip, QTimer *, showCountdownTimer);
TEST_F(CountdownTooltipTest, update)
{
    access_private_field::CountdownTooltipshowCountdownCounter(*m_countTip) = 1;
    access_private_field::CountdownTooltipshowCountdownTimer(*m_countTip) = new QTimer();
//    connect(access_private_field::CountdownTooltipshowCountdownTimer(*m_countTip), SIGNAL(timeout()), this, SLOT(update()));
//    access_private_field::CountdownTooltipshowCountdownTimer(*m_countTip)->start(1000);
    m_countTip->update();
    delete access_private_field::CountdownTooltipshowCountdownTimer(*m_countTip);
}
