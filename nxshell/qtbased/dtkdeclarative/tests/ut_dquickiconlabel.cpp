// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dquickiconlabel_p.h"

#include <QQuickItem>
#include <private/qquicktext_p.h>

DQUICK_USE_NAMESPACE

class ut_DQuickIconLabel : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/IconLabel.qml"));

        target = helper.object->findChild<DQuickIconLabel *>();
        ASSERT_TRUE(target);
    }

    ControlHelper<QQuickItem> helper;
    DQuickIconLabel *target = nullptr;
};

TEST_F(ut_DQuickIconLabel, properties)
{
    EXPECT_EQ(target->icon().width(), 20);
    EXPECT_EQ(target->icon().height(), 20);
    EXPECT_EQ(target->icon().name(), "switch_button");
    EXPECT_EQ(target->text(), "test");
    EXPECT_EQ(target->font().pixelSize(), 17);
    EXPECT_EQ(target->color(), Qt::red);
    EXPECT_EQ(target->display(), DQuickIconLabel::IconBesideText);
    EXPECT_EQ(target->spacing(), 2);
    EXPECT_EQ(target->isMirrored(), false);
    EXPECT_EQ(target->alignment(), 129);
    EXPECT_EQ(target->topPadding(), 1);
    target->resetTopPadding();
    EXPECT_EQ(target->topPadding(), 0);
    EXPECT_EQ(target->leftPadding(), 1);
    target->resetLeftPadding();
    EXPECT_EQ(target->leftPadding(), 0);
    EXPECT_EQ(target->rightPadding(), 1);
    target->resetRightPadding();
    EXPECT_EQ(target->rightPadding(), 0);
    EXPECT_EQ(target->bottomPadding(), 1);
    target->resetBottomPadding();
    EXPECT_EQ(target->bottomPadding(), 0);
}

TEST_F(ut_DQuickIconLabel, displayIconOnly)
{
    target->setDisplay(DQuickIconLabel::IconOnly);

    auto image = findItem<DQuickDciIconImage>(helper.object);
    EXPECT_EQ(image->x(), 1);
    auto text = findItem<QQuickText>(helper.object);
    EXPECT_FALSE(text);
}

TEST_F(ut_DQuickIconLabel, displayTextOnly)
{
    target->setDisplay(DQuickIconLabel::TextOnly);

    auto image = findItem<DQuickDciIconImage>(helper.object);
    EXPECT_FALSE(image);
    auto text = findItem<QQuickText>(helper.object);
    EXPECT_EQ(text->x(), 1);
}

TEST_F(ut_DQuickIconLabel, displayIconBesideText)
{
    target->setDisplay(DQuickIconLabel::IconBesideText);

    auto image = findItem<DQuickDciIconImage>(helper.object);
    ASSERT_TRUE(image);
    auto text = findItem<QQuickText>(helper.object);
    ASSERT_TRUE(text);

    EXPECT_GT(image->position().x(), text->position().x());
}

TEST_F(ut_DQuickIconLabel, displayTextBesideIcon)
{
    target->setDisplay(DQuickIconLabel::TextBesideIcon);

    auto image = findItem<DQuickDciIconImage>(helper.object);
    ASSERT_TRUE(image);
    auto text = findItem<QQuickText>(helper.object);
    ASSERT_TRUE(text);

    EXPECT_LT(image->position().x(), text->position().x());
}

TEST_F(ut_DQuickIconLabel, displayTextUnderIcon)
{
    target->setDisplay(DQuickIconLabel::TextUnderIcon);

    auto image = findItem<DQuickDciIconImage>(helper.object);
    EXPECT_TRUE(image);
    auto text = findItem<QQuickText>(helper.object);
    EXPECT_TRUE(text);

    EXPECT_LT(image->position().y(), text->position().y());
}
