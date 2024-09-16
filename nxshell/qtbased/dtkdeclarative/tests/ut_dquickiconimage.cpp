// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dquickiconimage_p.h"

#include <QQuickItem>
#include <QSignalSpy>
#include <private/qquicktext_p.h>

DQUICK_USE_NAMESPACE

TEST(ut_DQuickIconImage, properties)
{
    ControlHelper<DQuickIconImage> helper("qrc:/qml/QtIcon.qml");
    ASSERT_TRUE(helper.object);

    DQuickIconImage *target = helper.object;
    EXPECT_EQ(target->color(), Qt::blue);
    EXPECT_EQ(target->name(), "qrc:/icon/red.svg");
    EXPECT_EQ(target->state(), DQuickIconImage::State::On);
    EXPECT_EQ(target->mode(), DQuickIconImage::Mode::Active);
    EXPECT_EQ(target->fallbackSource(), QUrl("qrc:/icon/red.svg"));
    {
        QSignalSpy spy(target, &DQuickIconImage::nameChanged);
        target->setName("search_indicator");
        ASSERT_EQ(spy.count(), 1);
    }
    {
        QSignalSpy spy(target, &DQuickIconImage::stateChanged);
        target->setState(DQuickIconImage::State::Off);
        ASSERT_EQ(spy.count(), 1);
    }
    {
        QSignalSpy spy(target, &DQuickIconImage::modeChanged);
        target->setMode(DQuickIconImage::Mode::Disabled);
        ASSERT_EQ(spy.count(), 1);
    }
    {
        QSignalSpy spy(target, &DQuickIconImage::colorChanged);
        target->setColor(Qt::red);
        ASSERT_EQ(spy.count(), 1);
    }
    {
        QSignalSpy spy(target, &DQuickIconImage::fallbackSourceChanged);
        target->setFallbackSource(QUrl("test"));
        ASSERT_EQ(spy.count(), 1);
    }
}

TEST(ut_DQuickIconImage, image)
{
    QuickViewHelper<> helper("qrc:/qml/QtIcon.qml");
    ASSERT_TRUE(helper.object);

    auto windowImange = helper.object->window()->grabWindow();
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 50)), Qt::red);
}
