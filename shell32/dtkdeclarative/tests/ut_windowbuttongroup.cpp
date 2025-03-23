// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QSignalSpy>
#include <QQuickItem>
#include <QQuickWindow>

#include <DQuickWindow>
DQUICK_USE_NAMESPACE

class ut_WindowButtonGroup : public ::testing::Test
{
public:
};

TEST_F(ut_WindowButtonGroup, windowFlags)
{
    ControlHelper<QQuickWindow> helper("qrc:/qml/WindowButtonGroup.qml");
    ASSERT_TRUE(helper.object);

    QQuickItem *minimizeBtn = helper.object->findChild<QQuickItem *>("minimizeBtn");
    ASSERT_FALSE(minimizeBtn->property("hasWindowFlag").toBool());

    QQuickItem *maxOrWindedBtn = helper.object->findChild<QQuickItem *>("maxOrWindedBtn");
    ASSERT_TRUE(maxOrWindedBtn->property("hasWindowFlag").toBool());

    QQuickItem *closeBtn = helper.object->findChild<QQuickItem *>("closeBtn");
    ASSERT_TRUE(closeBtn->property("hasWindowFlag").toBool());
}

TEST_F(ut_WindowButtonGroup, maxOrWinded)
{
    ControlHelper<QQuickWindow> helper("qrc:/qml/WindowButtonGroup.qml");
    ASSERT_TRUE(helper.object);
    helper.requestExposed();

    auto dwAttached = qobject_cast<DQuickWindowAttached *>(qmlAttachedPropertiesObject<DQuickWindow>(helper.object, false));
    ASSERT_TRUE(dwAttached);
    if(!dwAttached->property("enabled").toBool())
        GTEST_SKIP();

    auto content = qvariant_cast<QObject *>(helper.object->property("group"));
    QSignalSpy maxOrWindedSpy(content, SIGNAL(maxOrWinded()));

    QQuickItem *maxOrWindedBtn = helper.object->findChild<QQuickItem *>("maxOrWindedBtn");

    ASSERT_TRUE(maxOrWindedBtn->isVisible());

    QTest::mouseClick(helper.object, Qt::LeftButton, Qt::KeyboardModifiers(), maxOrWindedBtn->mapToItem(helper.object->contentItem(), QPoint(10, 10)).toPoint());

    ASSERT_TRUE(QTest::qWaitFor([&maxOrWindedSpy]() {
        return maxOrWindedSpy.count() >= 1;
    }));
}
