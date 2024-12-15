// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>

#include "dquickrectangle_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickRectangle, properties)
{
    ControlHelper<DQuickRectangle> helper("qrc:/qml/DQuickRectangle.qml");
    ASSERT_TRUE(helper.object);

    EXPECT_EQ(helper.object->color(), Qt::red);
    EXPECT_EQ(helper.object->radius(), 8);
    EXPECT_EQ(helper.object->corners(), DQuickRectangle::TopLeftCorner | DQuickRectangle::TopRightCorner);
}

TEST(ut_DQuickRectangle, updateProperties)
{
    TEST_OFFSCREEN_SKIP();

    QuickViewHelper<DQuickRectangle> helper("qrc:/qml/DQuickRectangle.qml");
    ASSERT_TRUE(helper.object);

    helper.object->setRadius(16);
    helper.object->setColor(Qt::blue);
    helper.object->setCorners(DQuickRectangle::TopLeftCorner | DQuickRectangle::TopRightCorner);

    auto windowImange = helper.object->window()->grabWindow();

    const QPoint inCornerPoint(3, 3);
    const QPoint outCornerPoint(helper.object->boundingRect().center().toPoint());
    EXPECT_EQ(windowImange.pixelColor(outCornerPoint), Qt::blue);
    EXPECT_NE(windowImange.pixelColor(inCornerPoint), Qt::blue);
}
