// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>

#include "dsettingscontainer_p.h"
DQUICK_USE_NAMESPACE

class ut_SettingsContainer : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/SettingsContainer.qml"));
        target = helper.object;
    }

    ControlHelper<SettingsContainer> helper;
    SettingsContainer *target = nullptr;
};

TEST_F(ut_SettingsContainer, containerProperties)
{
    QQmlListReference rootGroups(target, "groups");
    ASSERT_EQ(rootGroups.count(), 2);
    ASSERT_EQ(target->groupList().size(), 3);

    ASSERT_TRUE(target->navigationTitle());
    ASSERT_TRUE(target->contentTitle());
    ASSERT_TRUE(target->contentBackground());

    ASSERT_TRUE(target->contentModel());
    ASSERT_TRUE(target->navigationModel());

    auto groups = target->groupList();
    auto it1 = std::find_if(groups.begin(), groups.end(), [this](const SettingsGroup *group) {
        return group->key() == "key-group1";
    });
    ASSERT_TRUE(it1 != groups.end());
}

TEST_F(ut_SettingsContainer, containerContentModel)
{
    SettingsContentModel *contentModel = target->contentModel();
    ASSERT_TRUE(contentModel);

    ASSERT_TRUE(contentModel->isValid());
    ASSERT_EQ(contentModel->count(), 3);

    QQmlListReference groups(target, "groups");
    SettingsGroup *group1 = qobject_cast<SettingsGroup *>(groups.at(0));
    ASSERT_EQ(contentModel->contentBackgroundByGroup(group1), target->contentBackground());
    SettingsGroup *group2 = qobject_cast<SettingsGroup *>(groups.at(1));
    ASSERT_NE(contentModel->contentBackgroundByGroup(group2), target->contentBackground());
}

TEST_F(ut_SettingsContainer, containerNavigationModel)
{
    SettingsNavigationModel *model = target->navigationModel();
    ASSERT_TRUE(model);

    ASSERT_TRUE(model->isValid());
    ASSERT_EQ(model->count(), 3);
}

TEST_F(ut_SettingsContainer, groupProperties)
{
    QQmlListReference groups(target, "groups");
    SettingsGroup *group1 = qobject_cast<SettingsGroup *>(groups.at(0));
    ASSERT_TRUE(group1);

    EXPECT_EQ(group1->name(), "group1");
    ASSERT_EQ(group1->level(), 0);

    QQmlListReference children(group1, "children");
    EXPECT_EQ(children.count(), 1);

    QQmlListReference options(group1, "options");
    ASSERT_EQ(options.count(), 2);
}

TEST_F(ut_SettingsContainer, optionProperties)
{
    QQmlListReference groups(target, "groups");
    SettingsGroup *group1 = qobject_cast<SettingsGroup *>(groups.at(0));
    ASSERT_TRUE(group1);
    QQmlListReference options(group1, "options");
    SettingsOption *option1 = qobject_cast<SettingsOption *>(options.at(0));
    ASSERT_TRUE(option1);

    EXPECT_EQ(option1->key(), "key-option1");
    EXPECT_EQ(option1->name(), "option1");

    ASSERT_TRUE(option1->delegate());
}

TEST_F(ut_SettingsContainer, groupVislble)
{
    QQmlListReference groups(target, "groups");
    SettingsGroup *group1 = qobject_cast<SettingsGroup *>(groups.at(0));
    ASSERT_TRUE(group1);
    QQmlListReference children(group1, "children");
    SettingsGroup *group11 = qobject_cast<SettingsGroup *>(children.at(0));
    ASSERT_TRUE(group11);

    ASSERT_EQ(target->groupVisible("key-group1"), true);
    ASSERT_EQ(group11->visible(), true);
    target->setGroupVisible("key-group1", false);
    ASSERT_EQ(target->groupVisible("key-group1"), false);
    ASSERT_EQ(group11->visible(), false);
}

class ut_SettingsContainerView : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(helper.load("qrc:/qml/SettingsContainer_View.qml"));
        target = helper.object;

        QQmlListReference groups(target, "groups");
        group1 = qobject_cast<SettingsGroup *>(groups.at(0));
        ASSERT_TRUE(group1);
        QQmlListReference options(group1, "options");
        option1 = qobject_cast<SettingsOption *>(options.at(0));
        ASSERT_TRUE(option1);
    }

    ControlHelper<SettingsContainer> helper;
    SettingsContainer *target = nullptr;
    SettingsGroup *group1 = nullptr;
    SettingsOption *option1 = nullptr;
};

TEST_F(ut_SettingsContainerView, navigationGroupAttached)
{
    SettingsNavigationModel *model = target->navigationModel();
    ASSERT_TRUE(model);
    auto object1 = model->object(0, QQmlIncubator::Synchronous);
    ASSERT_TRUE(object1);

    auto attached1 = qobject_cast<SettingsGroup *>(qmlAttachedPropertiesObject<SettingsGroup>(object1, false));
    ASSERT_TRUE(attached1);

    ASSERT_EQ(attached1, group1);
}

TEST_F(ut_SettingsContainerView, contentObject)
{
    SettingsContentModel *model = target->contentModel();
    ASSERT_TRUE(model);
    auto object1 = model->object(0, QQmlIncubator::Synchronous);
    ASSERT_TRUE(object1);
}
