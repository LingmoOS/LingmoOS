// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickglow_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickGlow, properties)
{
    ControlHelper<> helper("qrc:/qml/GlowEffect.qml");
    ASSERT_TRUE(helper.object);

    DQuickGlow *target = helper.object->findChild<DQuickGlow *>();
    EXPECT_EQ(target->glowRadius(), 50);
    EXPECT_EQ(target->color(), Qt::red);
    EXPECT_EQ(target->spread(), 0.01);
    EXPECT_EQ(target->relativeSizeX(), 0.5);
    EXPECT_EQ(target->relativeSizeY(), 0.5);
    EXPECT_EQ(target->fill(), true);
}

TEST(ut_DQuickGlow, updatePaintNode)
{
    TEST_SOFTWARE_SKIP();

    QuickViewHelper<> helper("qrc:/qml/GlowEffect.qml");
    ASSERT_TRUE(helper.object);

    auto windowImange = helper.object->window()->grabWindow();

    // center
    EXPECT_EQ(windowImange.pixelColor(QPoint(100, 100)), Qt::red);

    // rectangle
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 50)), Qt::blue);
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 150)), Qt::blue);
    EXPECT_EQ(windowImange.pixelColor(QPoint(150, 50)), Qt::blue);
    EXPECT_EQ(windowImange.pixelColor(QPoint(150, 150)), Qt::blue);

    // circle
    EXPECT_NE(windowImange.pixelColor(QPoint(75, 100)), Qt::blue);
    EXPECT_NE(windowImange.pixelColor(QPoint(100, 75)), Qt::blue);
    EXPECT_NE(windowImange.pixelColor(QPoint(125, 75)), Qt::blue);
    EXPECT_NE(windowImange.pixelColor(QPoint(100, 125)), Qt::blue);
}
