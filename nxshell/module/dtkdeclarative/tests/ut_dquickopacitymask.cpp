// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dquickopacitymask_p.h"

#include <QQuickItem>

DQUICK_USE_NAMESPACE

TEST(ut_DQuickOpacityMask, properties)
{
    ControlHelper<> helper("qrc:/qml/SoftwareOpacityMask.qml");
    ASSERT_TRUE(helper.object);

    DQuickOpacityMask *target = helper.object->findChild<DQuickOpacityMask *>();
    ASSERT_TRUE(target);

    QQuickItem *rectItem1 = helper.object->findChild<QQuickItem *>("rect1");
    ASSERT_TRUE(rectItem1);

    QQuickItem *rectItem2 = helper.object->findChild<QQuickItem *>("rect2");
    ASSERT_TRUE(rectItem2);

    EXPECT_EQ(target->source(), rectItem1);
    EXPECT_EQ(target->maskSource(), rectItem2);
    EXPECT_EQ(target->invert(), true);
}

TEST(ut_DQuickOpacityMask, image)
{
    TEST_NOTSOFTWARE_SKIP();

    QuickViewHelper<> helper("qrc:/qml/SoftwareOpacityMask.qml");
    ASSERT_TRUE(helper.object);

    auto img = helper.object->window()->grabWindow();
    EXPECT_NE(img.pixelColor(QPoint(125, 125)), Qt::red); //TODO: render abnormal!!!
}
