// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QEventLoop>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QPixmap>
#include <QPaintEvent>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/button_feedback.h"

using namespace testing;

class ButtonFeedbackTest: public testing::Test
{

public:
    Stub stub;
    ButtonFeedback *buttonFeedback;
    virtual void SetUp() override
    {
        buttonFeedback = new ButtonFeedback();
        std::cout << "start ButtonFeedbackTest" << std::endl;
    }

    virtual void TearDown() override
    {
        delete buttonFeedback;
        std::cout << "end ButtonFeedbackTest" << std::endl;
    }
};

ACCESS_PRIVATE_FIELD(ButtonFeedback, QTimer *, timer);
ACCESS_PRIVATE_FIELD(ButtonFeedback, int, frameIndex);
TEST_F(ButtonFeedbackTest, update)
{
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = new QTimer();
    access_private_field::ButtonFeedbackframeIndex(*buttonFeedback) = 9;
    buttonFeedback->update();
    delete access_private_field::ButtonFeedbacktimer(*buttonFeedback);
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = nullptr;
}

ACCESS_PRIVATE_FUN(ButtonFeedback, void(QPaintEvent *event), paintEvent);
ACCESS_PRIVATE_FIELD(ButtonFeedback, QPainter *, m_painter);
//ACCESS_PRIVATE_FIELD(ButtonFeedback, QPixmap[10], buttonFeedbackImg);

void accept_stub()
{

}
TEST_F(ButtonFeedbackTest, paintEvent)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set(ADDR(QPaintEvent, accept), accept_stub);
    access_private_field::ButtonFeedbackframeIndex(*buttonFeedback) = 0;
//    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
//    access_private_field::ButtonFeedbackbuttonFeedbackImg(*buttonFeedback)[0] = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);
    QRect rect(0, 0, 1920, 1080);
    QPaintEvent *event = new QPaintEvent(rect);
    call_private_fun::ButtonFeedbackpaintEvent(*buttonFeedback, event);

    stub.reset(ADDR(QPaintEvent, accept));
    delete event;
}

TEST_F(ButtonFeedbackTest, showPressFeedback)
{
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = new QTimer();
    buttonFeedback->showPressFeedback(50, 150);
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    access_private_field::ButtonFeedbacktimer(*buttonFeedback)->stop();
    delete access_private_field::ButtonFeedbacktimer(*buttonFeedback);
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = nullptr;

}

TEST_F(ButtonFeedbackTest, showDragFeedback)
{
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = new QTimer();
    buttonFeedback->showDragFeedback(50, 150);
    access_private_field::ButtonFeedbacktimer(*buttonFeedback)->start();
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    delete access_private_field::ButtonFeedbacktimer(*buttonFeedback);
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = nullptr;

}

TEST_F(ButtonFeedbackTest, showReleaseFeedback)
{

    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = new QTimer();
    buttonFeedback->showReleaseFeedback(50, 150);
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    access_private_field::ButtonFeedbacktimer(*buttonFeedback)->stop();
    delete access_private_field::ButtonFeedbacktimer(*buttonFeedback);
    access_private_field::ButtonFeedbacktimer(*buttonFeedback) = nullptr;

}
