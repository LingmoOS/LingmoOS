// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>

#include "dquickarrowboxpath_p.h"
DQUICK_USE_NAMESPACE

class ut_DQuickArrowBoxPath : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/ArrowBoxPath.qml"));

        target = helper.object->findChild<DQuickArrowBoxPath *>();
        ASSERT_TRUE(target);
    }

    ControlHelper<QQuickItem> helper;
    DQuickArrowBoxPath *target = nullptr;
};

TEST_F(ut_DQuickArrowBoxPath, properties)
{
    EXPECT_EQ(target->width(), 100);
    EXPECT_EQ(target->arrowX(), 50);
    EXPECT_EQ(target->arrowY(), 10);
    EXPECT_EQ(target->arrowWidth(), 20);
    EXPECT_EQ(target->arrowHeight(), 20);
    EXPECT_EQ(target->arrowDirection(), DQuickArrowBoxPath::Up);
    EXPECT_EQ(target->roundedRadius(), 8);
}

TEST_F(ut_DQuickArrowBoxPath, updateProperties)
{
    target->setArrowDirection(DQuickArrowBoxPath::Down);
    EXPECT_EQ(target->arrowDirection(), DQuickArrowBoxPath::Down);

    target->setSpread(5);
    EXPECT_EQ(target->spread(), 5);
}

TEST(ut_DQuickArrowBoxPath2, shapes)
{
    QuickViewHelper<> helper("qrc:/qml/ArrowBoxPath.qml");
    ASSERT_TRUE(helper.object);

    auto windowImange = helper.object->window()->grabWindow();

    const QColor inArrowColor(Qt::blue);
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 12)), inArrowColor);
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 20)), inArrowColor);

    const QColor outsideArrowColor(Qt::red);
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 9)), outsideArrowColor);
    EXPECT_EQ(windowImange.pixelColor(QPoint(10, 10)), outsideArrowColor);
}
