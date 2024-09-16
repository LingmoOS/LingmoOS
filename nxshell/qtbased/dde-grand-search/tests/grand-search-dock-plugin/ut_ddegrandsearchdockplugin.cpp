// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "ddegrandsearchdockplugin.h"
#include "ut_mock_pluginproxyinterface.h"
#include "gui/grandsearchwidget.h"
#include "gui/tipswidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DApplication>
#include <QGSettings>
#include <QLabel>

using namespace testing;
DWIDGET_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE

namespace  {
    class TestDdeGrandSearchDockPlugin : public testing::Test {
    public:

        void SetUp() override
        {
            stu.set_lamda(&DApplication::loadTranslator, [&]() {
                return true;
            });

            plugin.reset( new DdeGrandSearchDockPlugin);
            plugin->init(&mock_proxy);
        }
        void TearDown() override
        {
        }
    public:
        MockPluginProxyInterface mock_proxy;
        std::shared_ptr<DdeGrandSearchDockPlugin> plugin;
        stub_ext::StubExt stu;
    };
}

TEST_F(TestDdeGrandSearchDockPlugin, constructor)
{
    EXPECT_TRUE(plugin);
}

TEST_F(TestDdeGrandSearchDockPlugin, pluginName)
{
    QString result = plugin->pluginName();

    EXPECT_EQ(result, QString("grand-search"));
}

TEST_F(TestDdeGrandSearchDockPlugin, pluginDisplayName)
{
    QString result = plugin->pluginDisplayName();

    EXPECT_EQ(result, QString("Grand Search"));
}

TEST_F(TestDdeGrandSearchDockPlugin, init)
{
    EXPECT_NE(plugin->m_tipsWidget.get(), nullptr);
    EXPECT_NE(plugin->m_searchWidget.get(), nullptr);
    if (QGSettings::isSchemaInstalled("com.deepin.dde.dock.module.grand-search"))
        EXPECT_NE(plugin->m_gsettings.get(), nullptr);
    else
        EXPECT_EQ(plugin->m_gsettings.get(), nullptr);
}

TEST_F(TestDdeGrandSearchDockPlugin, itemWidget)
{
    QWidget *result = plugin->itemWidget(QString("test"));
    EXPECT_EQ(result, plugin->m_searchWidget.data());
}

TEST_F(TestDdeGrandSearchDockPlugin, itemTipsWidget)
{
    TipsWidget *result = qobject_cast<TipsWidget *>(plugin->itemTipsWidget(QString("test")));
    EXPECT_EQ(result, plugin->m_tipsWidget.get());
}

TEST_F(TestDdeGrandSearchDockPlugin, pluginIsAllowDisable)
{
    bool result = plugin->pluginIsAllowDisable();
    EXPECT_TRUE(result);
}

TEST_F(TestDdeGrandSearchDockPlugin, pluginIsDisable)
{
    EXPECT_CALL(mock_proxy, getValue).Times(1).WillRepeatedly(testing::Return(false));

    bool result = plugin->pluginIsDisable();

    EXPECT_FALSE(result);
}

TEST_F(TestDdeGrandSearchDockPlugin, pluginStateSwitched)
{
    EXPECT_NO_FATAL_FAILURE(plugin->pluginStateSwitched());
}

TEST_F(TestDdeGrandSearchDockPlugin, itemCommand)
{
    QString itemKey;

    QString result = plugin->itemCommand(itemKey);
    EXPECT_TRUE(result.isEmpty());

    stub_ext::StubExt stu;

    QString ut_itemCommand("testCommand");

    stu.set_lamda(ADDR(GrandSearchWidget, itemCommand), [&](){
        return ut_itemCommand;
    });

    itemKey = "grand-search";
    result = plugin->itemCommand(itemKey);
    EXPECT_EQ(result, ut_itemCommand);
}

TEST_F(TestDdeGrandSearchDockPlugin, itemSortKey)
{
    EXPECT_CALL(mock_proxy,getValue).WillOnce(testing::Return(1));
    EXPECT_EQ(1, plugin->itemSortKey("dde-grand-search"));
}

TEST_F(TestDdeGrandSearchDockPlugin, setSortKey)
{
    EXPECT_CALL(mock_proxy,saveValue).Times(1);

    plugin->setSortKey("dde-grand-search",1);

    EXPECT_CALL(mock_proxy,getValue).WillOnce(testing::Return(1));

    EXPECT_EQ( plugin->itemSortKey("dde-grand-search"), 1);
}

TEST_F(TestDdeGrandSearchDockPlugin, itemContextMenu)
{
    QString testKey("testKey");

    QString result = plugin->itemContextMenu(testKey);
    EXPECT_TRUE(result.isEmpty());

    testKey = "grand-search";
    result = plugin->itemContextMenu(testKey);
    EXPECT_TRUE(result.contains("menu_open_setting"));
}

TEST_F(TestDdeGrandSearchDockPlugin, invokedMenuItem)
{
    bool ut_startDetached = false;

    stu.set_lamda((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), [&](){
       ut_startDetached = true;
       return ut_startDetached;
    });

    QString testKey("testKey");
    QString testId("menu_open_setting");
    bool checked = false;

    plugin->invokedMenuItem(testKey, testId, checked);
    EXPECT_FALSE(ut_startDetached);

    testKey = "grand-search";
    plugin->invokedMenuItem(testKey, testId, checked);
    EXPECT_TRUE(ut_startDetached);
}

TEST_F(TestDdeGrandSearchDockPlugin, onGsettingsChanged)
{
    QString testKey("menuEnable");
    if (plugin->m_gsettings.get())
        EXPECT_NO_FATAL_FAILURE(plugin->onGsettingsChanged(testKey));
}
