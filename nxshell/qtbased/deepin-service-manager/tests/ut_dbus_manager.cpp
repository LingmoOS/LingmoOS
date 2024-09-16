// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dbus_manager.h"

#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QPaintEvent>
#include <QSignalSpy>
#include <QTest>

// 项目当前的插件数量
#define CUR_PLUGIN_COUNT 2

class Test_DBusManager : public ::testing::Test
{
public:
    virtual void SetUp() override;
};

void Test_DBusManager::SetUp()
{
    // 切换路径，方便插件插件
    QDir dir = QDir::currentPath();
    dir.cdUp();
    dir.cd("src");
    dir.cd("dde-service-manager");
    QDir::setCurrent(dir.absolutePath());
}

TEST_F(Test_DBusManager, create_test)
{
    // 无法得知当前系统中确切有多少个插件
#if 0
    DBusManager *manager = DBusManager::instance(SessionType::Session);
    QSignalSpy spy(manager, &DBusManager::PluginLoaded);
    qApp->processEvents();

    qDebug() << "spy.count(): "<< spy.count();
    ASSERT_EQ(spy.count(), CUR_PLUGIN_COUNT);

    // 启动后，插件加载，并发出对应信号，参数为插件的name
    while (!spy.isEmpty()) {
        QList<QVariant> arguments = spy.takeFirst();
        ASSERT_TRUE(manager->PluginList().contains(arguments.at(0).toString()));
    }

    // 当前的插件数量应该为2
    ASSERT_EQ(manager->PluginList().size(), CUR_PLUGIN_COUNT);
#endif
}

TEST_F(Test_DBusManager, Load_test)
{
    DBusManager *manager = DBusManager::instance();
    int count = manager->PluginList().size();
    QStringList list = manager->PluginList();

    // 异常参数测试1
    manager->Load(QString());
    ASSERT_EQ(count, manager->PluginList().size());
    ASSERT_EQ(list, manager->PluginList());

    // 异常参数测试2
    manager->Load("/tmp");
    ASSERT_EQ(count, manager->PluginList().size());
    ASSERT_EQ(list, manager->PluginList());

    // 异常参数测试3
    manager->Load("/tmp/test.so");
    ASSERT_EQ(count, manager->PluginList().size());
    ASSERT_EQ(list, manager->PluginList());

    // 防重复加载测试
    QDir libdir(manager->getPluginPath(SessionType::Session));
    for (auto name : libdir.entryList(QStringList() << "*")) {
        manager->Load(name);
    }
    ASSERT_EQ(count, manager->PluginList().size());
}

TEST_F(Test_DBusManager, IsRunning_test)
{
    DBusManager *manager = DBusManager::instance();
    for (auto p : manager->PluginList()) {
        ASSERT_TRUE(manager->IsRunning(p));
    }

    // 无效值测试
    ASSERT_FALSE(manager->IsRunning(QString()));
    ASSERT_FALSE(manager->IsRunning("/tmp"));
    ASSERT_FALSE(manager->IsRunning("/tmp/test.so"));
}

TEST_F(Test_DBusManager, UnLoad_test)
{
    DBusManager *manager = DBusManager::instance();
    int count = manager->PluginList().size();
    QStringList list = manager->PluginList();

    // 异常参数测试1
    manager->UnLoad(QString());
    ASSERT_EQ(list, manager->PluginList());
    ASSERT_EQ(count, manager->PluginList().size());

    // 异常参数测试2
    manager->UnLoad("/tmp");
    ASSERT_EQ(list, manager->PluginList());
    ASSERT_EQ(count, manager->PluginList().size());

    // 异常参数测试3
    manager->UnLoad("/tmp/test.so");
    ASSERT_EQ(list, manager->PluginList());
    ASSERT_EQ(count, manager->PluginList().size());

    // 卸载信号检查
    QSignalSpy spy(manager, &DBusManager::PluginUnLoaded);

    // 卸载插件
    for (auto p : list) {
        manager->UnLoad(p);
        ASSERT_NE(list, manager->PluginList());
        count = qMax(count - 1, 0);
        ASSERT_EQ(count, manager->PluginList().size());

        // 插件从插件列表中移除，并发出对应信号
        ASSERT_FALSE(spy.isEmpty());
        QList<QVariant> arguments = spy.takeFirst();
        ASSERT_FALSE(manager->PluginList().contains(arguments.at(0).toString()));
    }

    ASSERT_EQ(manager->PluginList().size(), 0);

    // 重复卸载
    for (auto p : list) {
        manager->UnLoad(p);
        ASSERT_FALSE(manager->IsRunning(p));
        ASSERT_TRUE(manager->PluginList().size() == 0);
    }
}
