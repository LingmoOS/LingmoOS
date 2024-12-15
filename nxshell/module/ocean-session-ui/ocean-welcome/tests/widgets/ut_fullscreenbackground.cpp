// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#define protected public
#include "fullscreenbackground.h"
#undef private
#undef protected

#include <QApplication>
#include <QEvent>
#include <QWidget>
#include <QRect>
#include <QPaintEvent>
#include <QResizeEvent>

class TstFullscreenBackground : public testing::Test
{
public:
    void SetUp() override
    {
        fullscreenBackground = new FullscreenBackground();
    }

    void TearDown() override
    {
        delete fullscreenBackground;
        fullscreenBackground = nullptr;
    }

public:
    FullscreenBackground *fullscreenBackground = nullptr;
};

TEST_F(TstFullscreenBackground, coverageTest)
{
    QWidget *widget = new QWidget(fullscreenBackground);
    fullscreenBackground->setContent(widget);
    fullscreenBackground->contentVisible();
    fullscreenBackground->updateBackground("");
    fullscreenBackground->setContentVisible(true);

    if (!qApp->screens().isEmpty()) {
        fullscreenBackground->setScreen(qApp->screens().first());
        fullscreenBackground->updateGeometry();

        fullscreenBackground->show();

        QEvent event(QEvent::Enter);
        qApp->sendEvent(fullscreenBackground, &event);

        QResizeEvent aresizeEvent(QSize(100,100), QSize(200, 200));
        fullscreenBackground->resizeEvent(&aresizeEvent);

        fullscreenBackground->updateBackground("bg.jpg");
        fullscreenBackground->m_fakeBackground = fullscreenBackground->m_background;
        QPaintEvent paintEvent(QRect(0, 0, 100, 100));
        fullscreenBackground->paintEvent(&paintEvent);

        QEvent event1(QEvent::Leave);
        qApp->sendEvent(fullscreenBackground, &event1);
    }
}
