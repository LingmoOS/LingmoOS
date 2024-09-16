// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../src/window/modules/securitytools/datausage/datausagemodel.h"
#include "../src/window/modules/securitytools/datausage/datausagewidget.h"
#include "../src/window/modules/securitytools/datausage/speedwidget.h"
#include "ut_datausagemodel.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

#include "gtest/gtest.h"

using namespace testing;

DataUsageModelTest::DataUsageModelTest()
{
}

void DataUsageModelTest::SetUp()
{
}

void DataUsageModelTest::TearDown()
{
}

TEST_F(DataUsageModelTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl dataInterface;
    MockDbusInvokerImpl monitorInterface;
    MockGSettingsImpl settingsInterface;

    factory.setInvokerInteface("com.deepin.defender.datainterface", &dataInterface);

    QList<QVariant> rst;
    QMap<QString, QDBusMessage> dataInterfaceMessagesMap;

    // mock返回处理
    auto insertMockMessage = [&](QMap<QString, QDBusMessage> &map, const QString &invokeName, QList<QVariant> &mockOutput) {
        QDBusMessage msg;
        msg.setArguments(mockOutput);
        map.insert(invokeName, msg);
        mockOutput.clear();
    };

    // mock返回处理
    // 期望的接口调用返回在下面增加
    insertMockMessage(dataInterfaceMessagesMap, "getRemRegisterStatus", rst << true);

    // 遍历返回集，指定调用的返回内容
    for (auto iter = dataInterfaceMessagesMap.begin(); iter != dataInterfaceMessagesMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(monitorInterface, Invoke(invokeName, _, _)).WillRepeatedly(Return(dataInterfaceMessagesMap[invokeName]));
    }

    // 所有与接口的信号连接返回true
    // 如果需要检查信号发送，在本测试类中connect到信号槽，不要尝试连接到真实的dbus服务上
    EXPECT_CALL(dataInterface, Connect(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(monitorInterface, Connect(_, _, _)).WillRepeatedly(Return(true));

    // 所有Gsetting返回值，全部设为true
    // 注意，类的构造或调用中，如果发现非bool型返回应单独指定mock需要返回的结果
    EXPECT_CALL(settingsInterface, GetValue(_)).WillRepeatedly(Return(QVariant(true)));

    DataUsageModel model(&factory);
    model.getProcInfoList();

    SpeedWidget sw(&model);
    DataUsageWidget duw(&model);
    FlowWidget fw(&model);
    AppDataDetailDialog addDialog("", "", "", &model);
}
