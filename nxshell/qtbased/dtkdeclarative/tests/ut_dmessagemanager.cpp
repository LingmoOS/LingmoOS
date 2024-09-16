// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>
#include <QSignalSpy>

#include "dmessagemanager_p.h"
DQUICK_USE_NAMESPACE

class ut_MessageManager : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/MessageManager.qml"));
        root = helper.object;

        target = qobject_cast<MessageManager *>(qmlAttachedPropertiesObject<MessageManager>(root, false));
        ASSERT_TRUE(target);
    }

    ControlHelper<QQuickWindow> helper;
    QQuickWindow *root = nullptr;
    MessageManager *target = nullptr;
};

TEST_F(ut_MessageManager, properties)
{
    ASSERT_TRUE(target->window());
    ASSERT_TRUE(target->delegate());
    ASSERT_TRUE(target->layout());
    EXPECT_EQ(target->capacity(), 3);
    EXPECT_EQ(target->count(), 0);
}

TEST_F(ut_MessageManager, sendMessage)
{
    ASSERT_TRUE(target->sendMessage("msg1", "icon1", 1));
    EXPECT_EQ(target->count(), 1);
    EXPECT_TRUE(QTest::qWaitFor([this]() {
        return target->count() <= 0;
    }));

    ASSERT_TRUE(target->sendMessage("msg1", "icon1", 1));
    ASSERT_TRUE(target->sendMessage("msg2", "icon2", 1, "id2"));
    ASSERT_TRUE(target->sendMessage("msg3", "icon3", 1, "id2"));

    EXPECT_EQ(target->count(), 2);
    EXPECT_EQ(target->messages("id2").size(), 1);

    EXPECT_TRUE(QTest::qWaitFor([this]() {
        return target->count() <= 0;
    }));
}

TEST_F(ut_MessageManager, sendMessageByDelegate)
{
    QQmlComponent *delegate = root->property("containerCom").value<QQmlComponent *>();
    ASSERT_TRUE(delegate);
    QVariantMap argus;
    argus.insert("msg", QStringList{"msg1"});
    argus.insert("icon", QStringList{"icon1"});

    target->sendMessage(delegate, argus, 1, "id1");
    EXPECT_EQ(target->count(), 1);
    const auto messages = target->messages("id1");
    ASSERT_EQ(messages.size(), 1);
    const auto message = messages.at(0);
    EXPECT_EQ(message->panel()->property("content"), "msg1");
    EXPECT_EQ(message->panel()->property("iconName"), "icon1");
}

TEST_F(ut_MessageManager, messageContainer)
{
    ASSERT_TRUE(target->sendMessage("msg1", "icon1", 1, "id1"));

    EXPECT_EQ(target->count(), 1);
    const auto messages = target->messages("id1");
    ASSERT_EQ(messages.size(), 1);
    const auto message = messages.at(0);
    EXPECT_EQ(message->duration(), 1);
    EXPECT_EQ(message->panel()->property("content"), "msg1");
    EXPECT_EQ(message->panel()->property("iconName"), "icon1");

    target->close("id1");
    EXPECT_EQ(target->count(), 0);
}
