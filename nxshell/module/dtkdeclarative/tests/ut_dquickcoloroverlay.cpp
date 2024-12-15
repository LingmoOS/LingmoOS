// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dquickcoloroverlay_p.h"

#include <QQuickItem>

DQUICK_USE_NAMESPACE

TEST(ut_DQuickColorOverlay, properties)
{
    ControlHelper<> helper("qrc:/qml/SoftwareColorOverlay.qml");
    ASSERT_TRUE(helper.object);

    DQuickColorOverlay *target = helper.object->findChild<DQuickColorOverlay *>();
    ASSERT_TRUE(target);

    QQuickItem *sourceItem = helper.object->findChild<QQuickItem *>("image");
    ASSERT_TRUE(sourceItem);

    EXPECT_EQ(target->source(), sourceItem);
    EXPECT_EQ(target->color(), "red");
    EXPECT_EQ(target->cached(), true);
}

TEST(ut_DQuickColorOverlay, image)
{
    TEST_NOTSOFTWARE_SKIP();

    QuickViewHelper<> helper("qrc:/qml/SoftwareColorOverlay.qml");
    ASSERT_TRUE(helper.object);

    auto img = helper.object->window()->grabWindow();
    EXPECT_EQ(img.pixelColor(QPoint(25, 25)), Qt::red);
}
