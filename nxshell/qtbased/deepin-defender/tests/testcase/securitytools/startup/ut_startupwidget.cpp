// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../src/window/modules/securitytools/startup/startupwidget.h"
#include "ut_startupwidget.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

#include "gtest/gtest.h"

using namespace testing;

StartupWidgetTest::StartupWidgetTest()
{
}

void StartupWidgetTest::SetUp()
{
}

void StartupWidgetTest::TearDown()
{
}

TEST_F(StartupWidgetTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl dataInterface;
    MockDbusInvokerImpl monitorInterface;
    MockGSettingsImpl settingsInterface;

    factory.setInvokerInteface("com.deepin.defender.datainterface", &dataInterface);
    factory.setInvokerInteface("com.deepin.defender.MonitorNetFlow", &monitorInterface);
    factory.setSettinsInteface("test gsettings", &settingsInterface);

    QList<QVariant> rst;
    QMap<QString, QDBusMessage> messagesMap;

    // mock返回处理
    auto insertMessage = [&](const QString &invokeName, QList<QVariant> &output) {
        QDBusMessage msg;
        msg.setArguments(output);
        messagesMap.insert(invokeName, msg);
        output.clear();
    };

    // datainterface mock返回处理
    insertMessage("startLauncherManage", rst << true);
    insertMessage("getAppsInfoEnable", rst << "");
    insertMessage("getAppsInfoDisable", rst << "");
    insertMessage("isAutostart", rst << true);
    insertMessage("exeAutostart", rst << true);

    for (auto iter = messagesMap.begin(); iter != messagesMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(dataInterface, Invoke(invokeName, _, _)).WillRepeatedly(Return(messagesMap[invokeName]));
    }
    EXPECT_CALL(dataInterface, Connect(_, _, _)).WillRepeatedly(Return(true));

    // monitorinterface mock返回处理
    EXPECT_CALL(monitorInterface, Invoke(_, _, _)).WillRepeatedly(Return(QDBusMessage()));

    StartupWidget widget(&factory);
}
