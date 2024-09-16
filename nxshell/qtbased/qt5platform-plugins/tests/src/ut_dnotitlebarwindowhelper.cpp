// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QStringList>
#include <QDebug>

#include "dnotitlebarwindowhelper.h"

DPP_USE_NAMESPACE

class TDNoTitlebarWindowHelper : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWindow *window = nullptr;
    DNoTitlebarWindowHelper *helper = nullptr;
};

void TDNoTitlebarWindowHelper::SetUp()
{
    window = new QWindow;
    window->create();
    helper = new DNoTitlebarWindowHelper(window, window->winId());
}

void TDNoTitlebarWindowHelper::TearDown()
{
    delete window;
}

TEST_F(TDNoTitlebarWindowHelper, theme)
{
    QString theme = helper->property("theme").toString();
    ASSERT_EQ(theme, helper->theme());
    helper->setTheme("test");
    ASSERT_EQ("test", helper->theme());
    helper->setTheme("deepin");
    ASSERT_EQ("deepin", helper->theme());
}

TEST_F(TDNoTitlebarWindowHelper, windowRadius)
{
    helper->setWindowRadius(QPointF(0.1, 0.1));
    ASSERT_EQ(QPointF(0.1, 0.1), helper->windowRadius());
}

TEST_F(TDNoTitlebarWindowHelper, borderWidth)
{
    helper->setBorderWidth(2.0);
    ASSERT_EQ(2.0, helper->borderWidth());
}

TEST_F(TDNoTitlebarWindowHelper, borderColor)
{
    helper->setBorderColor(Qt::red);
    ASSERT_EQ(QColor(Qt::red), helper->borderColor());
}

TEST_F(TDNoTitlebarWindowHelper, shadowRadius)
{
    helper->setShadowRadius(2.0);
    ASSERT_EQ(2.0, helper->shadowRadius());
}

TEST_F(TDNoTitlebarWindowHelper, shadowOffset)
{
    helper->setShadowOffect(QPointF(1.0, 1.0));
    ASSERT_EQ(QPointF(1.0, 1.0), helper->shadowOffset());
}

TEST_F(TDNoTitlebarWindowHelper, shadowColor)
{
    helper->setShadowColor(Qt::blue);
    ASSERT_EQ(QColor(Qt::blue), helper->shadowColor());
}

TEST_F(TDNoTitlebarWindowHelper, mouseInputAreaMargins)
{
    helper->setMouseInputAreaMargins(QMarginsF(1.0, 1.0, 1.0 ,1.0));
    ASSERT_EQ(QMarginsF(1.0, 1.0, 1.0 ,1.0), helper->mouseInputAreaMargins());
}
