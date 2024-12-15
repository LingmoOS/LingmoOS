// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>
#include <QSignalSpy>
#include <QTimer>

#include "dquickwaterprogressattribute_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickWaterProgressAttribute, properties)
{
    QScopedPointer<QQuickItem> water(new QQuickItem);
    water->setSize(QSize(100, 100));
    water->setProperty("value", 10);

    DQuickWaterProgressAttribute attr;
    auto attrPops = attr.pops();
    EXPECT_GE(attrPops.count(&attrPops), 3);

    attr.setWaterProgress(water.data());
    ASSERT_EQ(attr.waterProgress(), water.data());

    attr.setFrontXOffset(10);
    ASSERT_EQ(attr.frontXOffset(), 10);

    attr.setImageWidth(100);
    ASSERT_EQ(attr.imageWidth(), 100);

    attr.setImageHeight(100);
    ASSERT_EQ(attr.imageHeight(), 100);

    WaterPopAttribute *pop = attrPops.at(&attrPops, 0);
    ASSERT_TRUE(pop);
    EXPECT_LT(pop->sizeRatio(), 1.0);
    EXPECT_LT(pop->width(), water->width());
    EXPECT_LT(pop->height(), water->height());

    const auto popX = pop->x();
    const auto popY = pop->y();
    const auto popYOffset = pop->yOffset();

    attr.setRunning(true);
    ASSERT_EQ(attr.running(), true);

    // trigger and process `timeout` for DQuickWaterProgressAttribute's timer.
    auto timer = attr.findChild<QTimer *>();
    ASSERT_TRUE(timer);
    // receive `timeout` signal.
    QSignalSpy spy(timer, SIGNAL(timeout()));
    EXPECT_TRUE(QTest::qWaitFor([this, &spy]() {
        return spy.count() == 1;
    }));

    EXPECT_NE(popX, pop->x());
    EXPECT_NE(popY, pop->y());
    EXPECT_NE(popYOffset, pop->yOffset());

    attr.setRunning(false);
    ASSERT_EQ(attr.running(), false);
}

TEST(ut_DQuickWaterProgressBar, waterProgressBar)
{
    ControlHelper<> root("qrc:/qml/WaterProgressBar.qml");
    ASSERT_TRUE(root.object);

    EXPECT_EQ(root.object->property("value"), 20);
    EXPECT_EQ(root.object->property("running"), true);

    auto attr = root.object->findChild<DQuickWaterProgressAttribute *>();
    ASSERT_TRUE(attr);
    auto attrPops = attr->pops();
    ASSERT_GE(attrPops.count(&attrPops), 1);

    WaterPopAttribute *pop = attrPops.at(&attrPops, 0);
    const auto popX = pop->x();

    root.object->setProperty("running", false);

    // stop update
    QTest::qWait(50);
    auto timer = attr->findChild<QTimer *>();
    ASSERT_TRUE(timer);
    // don't receive `timeout` signal.
    QSignalSpy spy(timer, SIGNAL(timeout()));
    EXPECT_TRUE(QTest::qWaitFor([this, &spy]() {
        return spy.count() <= 0;
    }));

    ASSERT_EQ(popX, pop->x());
}
