// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>
#include <QSignalSpy>

#include "dobjectmodelproxy_p.h"
DQUICK_USE_NAMESPACE

class ut_ObjectModelProxy : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/ObjectModelProxy.qml"));
        root = helper.object;

        target = helper.object->property("model").value<ObjectModelProxy *>();
        ASSERT_TRUE(target);
    }

    ControlHelper<QQuickItem> helper;
    QQuickItem *root = nullptr;
    ObjectModelProxy *target = nullptr;
};

TEST_F(ut_ObjectModelProxy, properties)
{
    EXPECT_EQ(target->count(), 3);
}

TEST_F(ut_ObjectModelProxy, filterAcceptsItem)
{
    EXPECT_EQ(root->property("count").toInt(), 3);
    root->setProperty("filterText", "Page2");
    target->update();
    ASSERT_EQ(root->property("count").toInt(), 1);

    root->setProperty("filterText", "");
    target->update();
    EXPECT_EQ(root->property("count").toInt(), 3);
}

TEST_F(ut_ObjectModelProxy, mappingWithSource)
{
    root->setProperty("filterText", "Page2");
    target->update();
    ASSERT_EQ(root->property("count").toInt(), 1);

    auto object1 = target->get(0);
    ASSERT_TRUE(object1);
    ASSERT_EQ(object1->property("text"), "Page2");

    auto sourceIndex1 = target->mapToSource(0);
    ASSERT_EQ(sourceIndex1, 1);
    auto index1 = target->mapFromSource(1);
    ASSERT_EQ(index1, 0);
}
