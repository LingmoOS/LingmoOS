// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QSignalSpy>
#include <QQuickItem>
#include <QQuickWindow>

#include <DQuickWindow>

DQUICK_USE_NAMESPACE

class ut_DQuickWindow : public ::testing::Test
{
public:
};

TEST_F(ut_DQuickWindow, enabled)
{
    TEST_OFFSCREEN_SKIP();

    ControlHelper<QQuickWindow> helper("qrc:/qml/DQuickWindow.qml");
    ASSERT_TRUE(helper.object);
    auto attached = qobject_cast<DQuickWindowAttached *>(qmlAttachedPropertiesObject<DQuickWindow>(helper.object, false));
    ASSERT_TRUE(attached);

    ASSERT_TRUE(attached->property("enabled").toBool());
}

TEST_F(ut_DQuickWindow, windowProperties)
{
    TEST_OFFSCREEN_SKIP();

    ControlHelper<QQuickWindow> helper("qrc:/qml/DQuickWindow.qml");
    ASSERT_TRUE(helper.object);

    auto attached = qobject_cast<DQuickWindowAttached *>(qmlAttachedPropertiesObject<DQuickWindow>(helper.object, false));
    ASSERT_TRUE(attached && attached->isEnabled());

    EXPECT_EQ(attached->window(), helper.object);

    EXPECT_EQ(attached->windowRadius(), 16);

    EXPECT_EQ(attached->borderWidth(), 1);

    EXPECT_EQ(attached->borderColor(), Qt::red);

    EXPECT_EQ(attached->shadowRadius(), 10);

    EXPECT_EQ(attached->shadowOffset(), QPoint(0, 2));

    EXPECT_EQ(attached->shadowColor(), Qt::red);

    EXPECT_EQ(attached->translucentBackground(), true);

    EXPECT_EQ(attached->enableSystemResize(), true);

    EXPECT_EQ(attached->enableSystemMove(), true);

    EXPECT_EQ(attached->enableBlurWindow(), true);

    EXPECT_EQ(attached->alphaBufferSize(), 8);
}
