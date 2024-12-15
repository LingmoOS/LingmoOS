// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "widgetmanager.h"
#include "instanceproxy.h"
#include "pluginspec.h"
#include "helper.hpp"

WIDGETS_FRAME_USE_NAMESPACE
static PluginGuard pluginGuard;
class ut_WidgetManager : public ::testing::Test
{
public:
};

TEST_F(ut_WidgetManager, matchVersion)
{
    ASSERT_EQ(WidgetManager::currentVersion(), QString("1.0"));
    ASSERT_TRUE(WidgetManager::matchVersion("1.1"));
    ASSERT_FALSE(WidgetManager::matchVersion("2.1"));
}

TEST_F(ut_WidgetManager, loadPlugins)
{
    WidgetManager manager;
    manager.loadPlugins();
    const auto &plugins = manager.plugins();
    ASSERT_TRUE(plugins.contains(ExamplePluginId));
    ASSERT_GE(plugins.size(), 1);

    ASSERT_TRUE(manager.getPlugin(ExamplePluginId));
    const auto &store = manager.loadWidgetStoreInstances();

    ASSERT_TRUE(store.contains(ExamplePluginId));
    ASSERT_EQ(store.values(ExamplePluginId).size(), 3);
}

TEST_F(ut_WidgetManager, createWidget)
{
    WidgetManager manager;
    manager.loadPlugins();
    auto instance = manager.createWidget(ExamplePluginId, IWidget::Middle);
    ASSERT_TRUE(instance);
    const auto &instanceId = instance->handler()->id();
    ASSERT_FALSE(instanceId.isEmpty());
    auto replacedInstance = manager.createWidget(ExamplePluginId, IWidget::Large, instanceId);
    ASSERT_EQ(instanceId, replacedInstance->handler()->id());
    instance->deleteLater();
    replacedInstance->deleteLater();
}

TEST_F(ut_WidgetManager, removeWidget)
{
    WidgetManager manager;
    manager.loadPlugins();
    auto instance = manager.createWidget(ExamplePluginId, IWidget::Middle);
    const auto &instanceId = instance->handler()->id();
    manager.removeWidget(instanceId);

    ASSERT_FALSE(manager.getInstance(instanceId));
}

static WidgetManager gManager;
class ut_WidgetPluginSpec : public ::testing::Test
{
public:
    static void SetUpTestCase() {
        gManager.loadPlugins();
    }
    static void TearDownTestCase() {
        gManager.shutdown();
    }

    virtual void SetUp() override
    {
        plugin = gManager.getPlugin(ExamplePluginId);
    }
    virtual void TearDown() override
    {
        plugin = nullptr;
    }
    WidgetPlugin *plugin = nullptr;
};

TEST_F(ut_WidgetPluginSpec, baseInfo)
{
    ASSERT_EQ(plugin->id(), ExamplePluginId);
    ASSERT_EQ(plugin->type(), IWidgetPlugin::Normal);
    ASSERT_EQ(plugin->supportTypes().size(), 3);

    const InstanceId instanceId("example-instance-id");
    auto instance = plugin->createWidget(IWidget::Large, instanceId);
    ASSERT_EQ(instance->handler()->id(), instanceId);
    delete instance;
}
