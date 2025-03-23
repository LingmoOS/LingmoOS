// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickbehindwindowblur_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickBehindWindowBlur, properties)
{
    QuickViewHelper<> helper("qrc:/qml/DQuickBehindWindowBlur.qml");
    ASSERT_TRUE(helper.object);

    DQuickBehindWindowBlur *target = helper.object->findChild<DQuickBehindWindowBlur *>();
    ASSERT_TRUE(target);

    if (!target->valid())
        GTEST_SKIP_("don't support BehindWindowBlur, it need WM support.");

    EXPECT_EQ(target->blendColor(), QColor(0, 255, 0, 0.4 * 255));
    EXPECT_EQ(target->cornerRadius(), 10);

    QQuickWindow *subWindow = helper.object->findChild<QQuickWindow *>();
    ASSERT_TRUE(subWindow);
    helper.requestExposed(subWindow);

    QImage img = helper.view->grabWindow();
    EXPECT_NE(img.pixelColor(QPoint(50, 50)), QColor(0, 255, 0, 0.4));
    EXPECT_EQ(img.pixelColor(QPoint(150, 50)), Qt::red);
}
