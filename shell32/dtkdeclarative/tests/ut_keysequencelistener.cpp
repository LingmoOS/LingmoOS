// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>
#include <QSignalSpy>

#include "dquickkeylistener_p.h"
DQUICK_USE_NAMESPACE

TEST(ut_DQuickKeyListener, properties)
{
    DQuickKeyListener target;
    QScopedPointer<QQuickItem> control(new QQuickItem);

    QSignalSpy targetSpy(&target, SIGNAL(targetChanged()));
    target.setTarget(control.data());
    ASSERT_EQ(target.target(), control.data());
    EXPECT_EQ(targetSpy.count(), 1);

    const QStringList keys{"Alt", "Ctrl"};
    target.setKeys(keys);
    EXPECT_EQ(target.keys(), keys);

    ASSERT_TRUE(QMetaObject::invokeMethod(&target, "clearKeys"));

    EXPECT_TRUE(target.keys().isEmpty());

    target.setMaxKeyCount(5);
    EXPECT_EQ(target.maxKeyCount(), 5);
}

TEST(ut_DQuickKeyListener, keyEvent)
{
    TEST_OFFSCREEN_SKIP();

    QuickViewHelper<> helper("qrc:/qml/KeySequenceEdit.qml");
    ASSERT_TRUE(helper.object);

    EXPECT_EQ(helper.object->property("keys").toStringList(), QStringList({"CTRL", "SHIFT"}));

    auto listener = helper.object->findChild<DQuickKeyListener *>(QString(), Qt::FindDirectChildrenOnly);
    ASSERT_TRUE(listener);

    helper.requestActivate();
    helper.object->forceActiveFocus();
    ASSERT_TRUE(helper.object->hasFocus());

    QSignalSpy listenerSpy(listener, SIGNAL(keysChanged()));
    QTest::keyClick(helper.object->window(), Qt::Key_Tab, Qt::ShiftModifier);

    ASSERT_EQ(listenerSpy.count(), 2);

    EXPECT_EQ(listener->keys().size(), 2);
    EXPECT_TRUE(listener->keys().contains("Shift"));
    EXPECT_TRUE(listener->keys().contains("Tab"));
    EXPECT_EQ(helper.object->property("keys").toStringList(), listener->keys());
}
