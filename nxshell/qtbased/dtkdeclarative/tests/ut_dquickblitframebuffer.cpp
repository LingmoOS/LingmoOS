// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickblitframebuffer.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickBlitFramebuffer, properties)
{
    QuickViewHelper<> helper("qrc:/qml/DQuickBlitFramebuffer.qml");
    ASSERT_TRUE(helper.object);

    auto target = findItem<DQuickBlitFramebuffer>(helper.object);
    ASSERT_TRUE(target);

    QImage img = helper.view->grabWindow();

    // out buffer area
    EXPECT_EQ(img.pixelColor(QPoint(50, 40)), Qt::red);
    // in buffer area
    EXPECT_EQ(img.pixelColor(QPoint(60, 60)), Qt::blue);
}
