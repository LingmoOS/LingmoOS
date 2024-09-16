// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dquickdciicon_p.h"

DQUICK_USE_NAMESPACE

class ut_DQuickDciIcon : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        target = new DQuickDciIcon;
    }

    virtual void TearDown()
    {
        delete target;
    }

    DQuickDciIcon *target = nullptr;
};

TEST_F(ut_DQuickDciIcon, name)
{
    EXPECT_TRUE(target->isEmpty());
    target->setName("test");
    EXPECT_EQ(target->name(), "test");
    target->resetName();
    EXPECT_EQ(target->name(), "");
}

TEST_F(ut_DQuickDciIcon, width)
{
    target->setWidth(20);
    EXPECT_EQ(target->width(), 20);
    target->resetWidth();
    EXPECT_EQ(target->width(), 0);
}

TEST_F(ut_DQuickDciIcon, height)
{
    target->setHeight(20);
    EXPECT_EQ(target->height(), 20);
    target->resetHeight();
    EXPECT_EQ(target->height(), 0);
}

TEST_F(ut_DQuickDciIcon, mode)
{
    target->setMode(DQMLGlobalObject::DisabledState);
    EXPECT_EQ(target->mode(), DQMLGlobalObject::DisabledState);
    target->resetMode();
    EXPECT_EQ(target->mode(), DQMLGlobalObject::NormalState);
}

TEST_F(ut_DQuickDciIcon, theme)
{
    target->setTheme(DGuiApplicationHelper::ColorType::DarkType);
    EXPECT_EQ(target->theme(), DGuiApplicationHelper::ColorType::DarkType);
    target->resetTheme();
    EXPECT_EQ(target->theme(), DGuiApplicationHelper::ColorType::LightType);
}

TEST_F(ut_DQuickDciIcon, palette)
{
    DDciIconPalette palette(Qt::red, Qt::green, Qt::red);
    target->setPalette(palette);
    EXPECT_EQ(target->palette(), palette);
    target->resetPalette();
    EXPECT_EQ(target->palette(), DDciIconPalette());
}

TEST_F(ut_DQuickDciIcon, source)
{
    QUrl source("qrc:/test");
    target->setSource(source);
    EXPECT_EQ(target->source(), source);
    target->resetSource();
    EXPECT_EQ(target->source(), QUrl());
}

TEST_F(ut_DQuickDciIcon, fallbackToQIcon)
{
    target->setFallbackToQIcon(false);
    EXPECT_EQ(target->fallbackToQIcon(), false);
    target->resetFallbackToQIcon();
    EXPECT_EQ(target->fallbackToQIcon(), true);
}

TEST_F(ut_DQuickDciIcon, operatorEQ)
{
    DQuickDciIcon dciIcon = *target;
    EXPECT_EQ(*target, dciIcon);
}
