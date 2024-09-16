// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
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
#include "../../src/widgets/scrollshottip.h"
#include "addr_pri.h"

using namespace testing;

class ScrollShotTipTest: public testing::Test
{

public:
    ScrollShotTip *scrollShotTip = new ScrollShotTip;
    virtual void SetUp() override
    {
        std::cout << "start ScrollShotTipTest" << std::endl;

    }

    virtual void TearDown() override
    {
        scrollShotTip->hide();
        delete scrollShotTip;
        std::cout << "end ScrollShotTipTest" << std::endl;
    }
public slots:
    void startRecord();
};

ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showStartScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showErrorScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showEndScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showMaxScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showQuickScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showInvalidAreaShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(QPainter &, QPixmap &, int), paintRect);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(QPainter &, QRect, QString, qreal, int), drawTooltipBackground);


TEST_F(ScrollShotTipTest, showTip)
{
    scrollShotTip->showTip(TipType::StartScrollShotTip);
    scrollShotTip->move(500, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::ErrorScrollShotTip);
    scrollShotTip->move(500, 500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::EndScrollShotTip);
    scrollShotTip->move(500, 500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::MaxLengthScrollShotTip);
    scrollShotTip->move(500, 500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

}
TEST_F(ScrollShotTipTest, getTipType)
{
    scrollShotTip->showTip(TipType::StartScrollShotTip);
    TipType tipType = scrollShotTip->getTipType();
    EXPECT_EQ(tipType, TipType::StartScrollShotTip);
}

TEST_F(ScrollShotTipTest, showStartScrollShotTip)
{
    call_private_fun::ScrollShotTipshowStartScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showErrorScrollShotTip)
{
    call_private_fun::ScrollShotTipshowErrorScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showEndScrollShotTip)
{
    call_private_fun::ScrollShotTipshowEndScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showMaxScrollShotTip)
{
    call_private_fun::ScrollShotTipshowMaxScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showQuickScrollShotTip)
{
    call_private_fun::ScrollShotTipshowQuickScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showInvalidAreaShotTip)
{
    call_private_fun::ScrollShotTipshowInvalidAreaShotTip(*scrollShotTip);
    scrollShotTip->move(1000, 500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}
TEST_F(ScrollShotTipTest, paintRect)
{
    QPainter painter;
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    QPixmap pix = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);
    call_private_fun::ScrollShotTippaintRect(*scrollShotTip, painter, pix, 20);

}

TEST_F(ScrollShotTipTest, drawTooltipBackground)
{
    QPainter painter;
    QRect rect(0, 0, 500, 500);
    QString textColor = "#191919";
    qreal opacity = 0.3;
    int radius = 20;
    call_private_fun::ScrollShotTipdrawTooltipBackground(*scrollShotTip, painter, rect, textColor, opacity, radius);

}
