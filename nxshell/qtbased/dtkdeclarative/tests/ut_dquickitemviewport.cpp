// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickitemviewport.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickItemViewport, properties)
{
    QuickViewHelper<> helper("qrc:/qml/DQuickItemViewport.qml");
    ASSERT_TRUE(helper.object);

    DQuickItemViewport *target = helper.object->findChild<DQuickItemViewport *>();
    ASSERT_TRUE(target);

    EXPECT_EQ(target->radius(), 10);
    EXPECT_TRUE(target->fixed());
    EXPECT_EQ(target->hideSource(), false);

    QImage img = helper.view->grabWindow();
    // first is source, and second is viewport.
    EXPECT_EQ(img.pixelColor(QPoint(50, 50)), img.pixelColor(QPoint(100 + 50, 100 + 50)));
}

TEST(ut_DQuickItemViewport, hideSource)
{
    QuickViewHelper<> helper("qrc:/qml/DQuickItemViewport_HideSource.qml");
    ASSERT_TRUE(helper.object);

    DQuickItemViewport *target = helper.object->findChild<DQuickItemViewport *>();
    ASSERT_TRUE(target);

    EXPECT_EQ(target->hideSource(), true);

    QImage img = helper.view->grabWindow();
    // first is hide, and second is viewport.
    EXPECT_EQ(img.pixelColor(QPoint(50, 50)), Qt::blue);
    EXPECT_NE(img.pixelColor(QPoint(50, 50)), img.pixelColor(QPoint(100 + 50, 100 + 50)));
}
