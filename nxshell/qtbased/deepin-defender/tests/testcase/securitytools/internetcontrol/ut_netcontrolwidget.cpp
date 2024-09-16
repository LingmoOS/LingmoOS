// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_netcontrolwidget.h"
#include "../src/window/modules/securitytools/internetcontrol/netcontrolwidget.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

using namespace testing;

NetControlWidgetTest::NetControlWidgetTest()
{
}

void NetControlWidgetTest::SetUp()
{
}

void NetControlWidgetTest::TearDown()
{
}

TEST_F(NetControlWidgetTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl dataInterface;
    MockDbusInvokerImpl monitorInterface;
    MockGSettingsImpl settingsInterface;

    factory.setInvokerInteface("com.deepin.defender.datainterface", &dataInterface);
    factory.setInvokerInteface("com.deepin.defender.MonitorNetFlow", &monitorInterface);
    factory.setSettinsInteface("com.deepin.dde.deepin-defender", &settingsInterface);

    QList<QVariant> rst;
    QMap<QString, QDBusMessage> monitorMessagesMap;
    QMap<QString, QDBusMessage> dataInterMessagesMap;

    // mock返回处理
    auto insertMockMessage = [&](QMap<QString, QDBusMessage> &map, const QString &invokeName, QList<QVariant> &mockOutput) {
        QDBusMessage msg;
        msg.setArguments(mockOutput);
        map.insert(invokeName, msg);
        mockOutput.clear();
    };

    // mock返回处理
    // 期望的接口调用返回在下面增加
    // insertMockMessage(monitorMessagesMap, "getRemRegisterStatus", rst << true);
    insertMockMessage(dataInterMessagesMap, "getNetAppsInfo", rst);
    insertMockMessage(dataInterMessagesMap, "setNetAppStatusChange", rst);

    // 遍历返回集，指定调用的返回内容
    for (auto iter = monitorMessagesMap.begin(); iter != monitorMessagesMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(monitorInterface, Invoke(invokeName, _, _)).WillRepeatedly(Return(monitorMessagesMap[invokeName]));
    }

    // 所有与接口的信号连接返回true
    // 如果需要检查信号发送，在本测试类中connect到信号槽，不要尝试连接到真实的dbus服务上
    EXPECT_CALL(dataInterface, Connect(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(monitorInterface, Connect(_, _, _)).WillRepeatedly(Return(true));

    // 所有Gsetting返回值，全部设为true
    // 注意，类的构造或调用中，如果发现非bool型返回应单独指定mock需要返回的结果
    EXPECT_CALL(settingsInterface, GetValue(_)).WillRepeatedly(Return(QVariant(false)));

    NetControlWidget widget("", &factory);
}
