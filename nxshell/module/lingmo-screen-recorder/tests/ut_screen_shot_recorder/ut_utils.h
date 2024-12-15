// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>
#include "../../src/utils.h"


using namespace testing;

class UtilsTest: public testing::Test
{

public:
    Utils *m_utils;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;
        m_utils = new Utils;

    }

    virtual void TearDown() override
    {
        delete m_utils;
        std::cout << "end UtilsTest" << std::endl;
    }
};
TEST_F(UtilsTest, setFontSize)
{

    QPainter painter;
    QFont font = painter.font() ;
    int start = font.pointSize();
    qDebug() << start;
    int setsize = 24;
    Utils::setFontSize(painter, setsize);
    //EXPECT_EQ(painter.font().pointSize(), setsize);
}

TEST_F(UtilsTest, getInputEvent)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(500, 500);
    mainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    mainWindow->setStyleSheet("border: 5px solid black;background-color: rgb(120, 120, 120)");
    mainWindow->move(500, 200);

    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    hBoxLayout->addStretch();
    QWidget *widget = new QWidget(mainWindow);
    widget->setStyleSheet("background-color: rgb(0, 120, 120);");
    widget->resize(200, 200);
    hBoxLayout->addWidget(widget);
    hBoxLayout->addStretch();
    QVBoxLayout *vBoxLayout = new QVBoxLayout(mainWindow);
    vBoxLayout->addStretch();
    vBoxLayout->addLayout(hBoxLayout);
    vBoxLayout->addStretch();
    mainWindow->show();

    Utils::getInputEvent(
        static_cast<int>(mainWindow->winId()),
        static_cast<short>(widget->x()),
        static_cast<short>(widget->y()),
        static_cast<unsigned short>(widget->width()),
        static_cast<unsigned short>(widget->height()));

    QPoint tempPoint(mainWindow->x() + mainWindow->width() / 2 - widget->width() / 2,
                     mainWindow->y() + mainWindow->height() / 2 - widget->height() / 2);

    QEventLoop loop;
    QTest::mouseMove(mainWindow, tempPoint);
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();

    mainWindow->close();
    delete hBoxLayout;
    delete vBoxLayout;
    delete mainWindow;


}

TEST_F(UtilsTest, cancelInputEvent)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(500, 500);
    mainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    mainWindow->setStyleSheet("border: 5px solid black;background-color: rgb(120, 120, 120)");
    mainWindow->move(500, 200);

    QHBoxLayout *hBoxLayout = new QHBoxLayout(mainWindow);
    hBoxLayout->addStretch();
    QWidget *widget = new QWidget(mainWindow);
    widget->setStyleSheet("background-color: rgb(0, 120, 120);");
    widget->resize(200, 200);
    hBoxLayout->addWidget(widget);
    hBoxLayout->addStretch();
    mainWindow->show();

    Utils::getInputEvent(
        static_cast<int>(mainWindow->winId()),
        static_cast<short>(widget->x()),
        static_cast<short>(widget->y()),
        static_cast<unsigned short>(widget->width()),
        static_cast<unsigned short>(widget->height()));
    Utils::cancelInputEvent(
        static_cast<int>(mainWindow->winId()),
        static_cast<short>(widget->x()),
        static_cast<short>(widget->y()),
        static_cast<unsigned short>(widget->width()),
        static_cast<unsigned short>(widget->height()));
    QPoint tempPoint(mainWindow->x() + mainWindow->width() / 2 - widget->width() / 2,
                     mainWindow->y() + mainWindow->height() / 2 - widget->height() / 2);
    QEventLoop loop;
    QTest::mouseMove(mainWindow, tempPoint);
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    mainWindow->close();
    delete hBoxLayout;
    delete mainWindow;
}

TEST_F(UtilsTest, cancelInputEvent1)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(500, 500);
    mainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    mainWindow->setStyleSheet("border: 5px solid black;background-color: rgb(120, 120, 120)");
    mainWindow->move(500, 200);

    QHBoxLayout *hBoxLayout = new QHBoxLayout(mainWindow);
    hBoxLayout->addStretch();
    QWidget *widget = new QWidget(mainWindow);
    widget->setStyleSheet("background-color: rgb(0, 120, 120);");
    widget->resize(200, 200);
    hBoxLayout->addWidget(widget);
    hBoxLayout->addStretch();
    mainWindow->show();

    Utils::getInputEvent(
        static_cast<int>(mainWindow->winId()),
        static_cast<short>(widget->x()),
        static_cast<short>(widget->y()),
        static_cast<unsigned short>(widget->width()),
        static_cast<unsigned short>(widget->height()));
    Utils::cancelInputEvent1(
        static_cast<int>(mainWindow->winId()),
        static_cast<short>(widget->x()),
        static_cast<short>(widget->y()),
        static_cast<unsigned short>(widget->width()),
        static_cast<unsigned short>(widget->height()));
    QPoint tempPoint(mainWindow->x() + mainWindow->width() / 2 - widget->width() / 2,
                     mainWindow->y() + mainWindow->height() / 2 - widget->height() / 2);
    QEventLoop loop;
    QTest::mouseMove(mainWindow, tempPoint);
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    mainWindow->close();
    delete hBoxLayout;
    delete mainWindow;
}

TEST_F(UtilsTest, enableXGrabButton)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(500, 500);
    mainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    mainWindow->setStyleSheet("border: 5px solid black;background-color: rgb(120, 120, 120)");
    mainWindow->move(500, 200);
    mainWindow->show();
    Utils::enableXGrabButton();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    mainWindow->close();
    Utils::disableXGrabButton();
    delete mainWindow;

}

TEST_F(UtilsTest, disableXGrabButton)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(500, 500);
    mainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    mainWindow->setStyleSheet("border: 5px solid black;background-color: rgb(120, 120, 120)");
    mainWindow->move(500, 200);
    mainWindow->show();
    Utils::enableXGrabButton();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    mainWindow->close();
    Utils::disableXGrabButton();
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    delete mainWindow;

}

TEST_F(UtilsTest, drawTooltipBackground)
{

    QPainter painter;
    QRect rect(0, 0, 500, 500);
    QString color = "#EBEBEB";
    qreal opacity = 0.8;
    m_utils->drawTooltipBackground(painter, rect, color, opacity);

}


TEST_F(UtilsTest, drawTooltipText)
{
    QPainter painter;
    QRect rect(0, 0, 500, 500);
    QString text = "this is test!";
    QString textColor = "#EBEBEB";
    int textSize = 10;
    m_utils->drawTooltipText(painter, text, textColor, textSize, rect);
}

