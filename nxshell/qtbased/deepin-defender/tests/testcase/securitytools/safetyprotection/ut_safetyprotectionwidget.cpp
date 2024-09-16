// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../src/window/modules/safetyprotection/safetyprotectionwidget.h"
#include "ut_safetyprotectionwidget.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

#include "gtest/gtest.h"

using namespace testing;

SafetyProtectionWidgetTest::SafetyProtectionWidgetTest()
{
}

void SafetyProtectionWidgetTest::SetUp()
{
    // 可以将所有的接口方法的期望返回结果全部写在这里
    // 在类中建立几个全局的map，进行统一管理
    // 或以map<map>形式管理不同用例中设计的输入
}

void SafetyProtectionWidgetTest::TearDown()
{
}

TEST_F(SafetyProtectionWidgetTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl dataInterface;
    MockDbusInvokerImpl monitorInterface;
    MockDbusInvokerImpl hmiInterface;
    MockGSettingsImpl settingsInterface;

    factory.setInvokerInteface("com.deepin.defender.datainterface", &dataInterface);
    factory.setInvokerInteface("com.deepin.defender.MonitorNetFlow", &monitorInterface);
    factory.setInvokerInteface("com.deepin.defender.hmiscreen", &hmiInterface);
    factory.setSettinsInteface("com.deepin.dde.deepin-defender", &settingsInterface);

    QList<QVariant> rst;
    QMap<QString, QDBusMessage> monitorMessagesMap;
    QMap<QString, QDBusMessage> dataMessagesMap;

    // mock返回处理
    auto insertMockMessage = [&](QMap<QString, QDBusMessage> &map, const QString &invokeName, QList<QVariant> &mockOutput) {
        QDBusMessage msg;
        msg.setArguments(mockOutput);
        map.insert(invokeName, msg);
        mockOutput.clear();
    };

    insertMockMessage(dataMessagesMap, "GetUosResourceStatus", QVariantList() << true);

    // mock返回处理
    // 期望的接口调用返回在下面增加

    // 遍历返回集，指定调用的返回内容
    for (auto iter = monitorMessagesMap.begin(); iter != monitorMessagesMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(monitorInterface, Invoke(invokeName, _, _)).WillRepeatedly(Return(monitorMessagesMap[invokeName]));
    }

    for (auto iter = dataMessagesMap.begin(); iter != dataMessagesMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(dataInterface, Invoke(invokeName, _, _)).WillRepeatedly(Return(dataMessagesMap[invokeName]));
    }

    // 所有与接口的信号连接返回true
    // 如果需要检查信号发送，在本测试类中connect到信号槽，不要尝试连接到真实的dbus服务上
    EXPECT_CALL(dataInterface, Connect(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(monitorInterface, Connect(_, _, _)).WillRepeatedly(Return(true));

    // 所有Gsetting返回值，全部设为true
    // 注意，类的构造或调用中，如果发现非bool型返回应单独指定mock需要返回的结果
    EXPECT_CALL(settingsInterface, GetValue(_)).WillRepeatedly(Return(QVariant(true)));

    SafetyProtectionWidget widget(&factory);
}
