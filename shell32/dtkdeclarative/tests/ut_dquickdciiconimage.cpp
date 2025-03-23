// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dquickdciiconimage_p.h"

#include <QQuickItem>

DQUICK_USE_NAMESPACE

TEST(ut_DQuickDciIconImage, properties)
{
    ControlHelper<> helper("qrc:/qml/DciIcon.qml");
    ASSERT_TRUE(helper.object);

    DQuickDciIconImage *target = helper.object->findChild<DQuickDciIconImage *>();
    EXPECT_EQ(target->name(), "switch_button");
    EXPECT_EQ(target->mode(), DTK_QUICK_NAMESPACE::DQMLGlobalObject::NormalState);
    EXPECT_EQ(target->theme(), DGuiApplicationHelper::LightType);
    EXPECT_EQ(target->sourceSize(), QSize(0, 0));
    EXPECT_EQ(target->mirror(), false);
    EXPECT_EQ(target->fallbackToQIcon(), true);
    QTest::qWait(1); // TODO: Resolve crash
    EXPECT_EQ(target->isNull("switch_button"), false);
}

TEST(ut_DQuickDciIconImage, image)
{
    QuickViewHelper<> helper("qrc:/qml/DciIcon.qml");
    ASSERT_TRUE(helper.object);

    auto windowImange = helper.object->window()->grabWindow();
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 50)), QColor(16, 131, 245)); // NOTICE: color ?
}
