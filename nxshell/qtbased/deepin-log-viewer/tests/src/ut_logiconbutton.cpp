// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logiconbutton.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QDebug>
#include <QEvent>
TEST(LogCombox_Constructor_UT, LogCombox_Constructor_UT_001)
{
    LogIconButton *p = new LogIconButton(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogCombox_Constructor_UT, LogCombox_Constructor_UT_002)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogCombox_sizeHint_UT, LogCombox_sizeHint_UT)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    p->sizeHint();
    p->deleteLater();
}

TEST(LogCombox_mousePressEvent_UT, LogCombox_mousePressEvent_UT)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    p->mousePressEvent(nullptr);
    p->deleteLater();
}

TEST(LogCombox_mousePressEvent_UT, LogCombox_mousePressEvent_UT_001)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    QEvent *e = new QEvent(QEvent::QueryWhatsThis);
    p->event(e);
    delete e;
    p->deleteLater();
}

TEST(LogCombox_mousePressEvent_UT, LogCombox_mousePressEvent_UT_002)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    QEvent *e = new QEvent(QEvent::HoverEnter);
    p->event(e);
    delete e;
    p->deleteLater();
}

TEST(LogCombox_resizeEvent_UT, LogCombox_resizeEvent_UT)
{
    LogIconButton *p = new LogIconButton("aa", nullptr);
    EXPECT_NE(p, nullptr);
    p->resizeEvent(nullptr);
    p->deleteLater();
}
