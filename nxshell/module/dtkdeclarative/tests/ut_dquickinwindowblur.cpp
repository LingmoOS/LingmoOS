// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickinwindowblur_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickInWindowBlur, properties)
{
    TEST_QRHI_SKIP();

    QuickViewHelper<> helper("qrc:/qml/DQuickInWindowBlur.qml");
    ASSERT_TRUE(helper.object);

    DQuickInWindowBlur *target = helper.object->findChild<DQuickInWindowBlur *>();
    ASSERT_TRUE(target);

    EXPECT_EQ(target->radius(), 10);

    QImage img = helper.view->grabWindow();
    // blured area
    EXPECT_TRUE(TestUtil::equalBlurColor(img.pixelColor(QPoint(50, 50)), TestUtil::simpleImage->pixelColor(QPoint(50, 50))));
    // don't be blured area
    EXPECT_NE(img.pixelColor(QPoint(90, 90)), TestUtil::simpleImage->pixelColor(QPoint(90, 90)));
}
