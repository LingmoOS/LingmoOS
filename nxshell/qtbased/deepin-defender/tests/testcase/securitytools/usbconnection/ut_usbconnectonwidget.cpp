// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_usbconnectonwidget.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

#include "../src/window/modules/securitytools/usbconnection/usbconnectionwork.h"
#include "../src/window/modules/securitytools/usbconnection/usbconnectionwidget.h"

using namespace testing;

UsbConnectionWidgetTest::UsbConnectionWidgetTest()
{
}

void UsbConnectionWidgetTest::SetUp()
{
}

void UsbConnectionWidgetTest::TearDown()
{
}

TEST_F(UsbConnectionWidgetTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl monitorNetFlowMockInter;

    factory.setInvokerInteface("com.deepin.defender.MonitorNetFlow", &monitorNetFlowMockInter);

    // monitorNetFlow mock返回处理
    auto insertMessage = [&](QMap<QString, QDBusMessage> &messagesMap, const QString &invokeName, QList<QVariant> &output) {
        QDBusMessage msg;
        msg.setArguments(output);
        messagesMap.insert(invokeName, msg);
        output.clear();
    };

    // datainterface mock返回处理
    QList<QVariant> rst;
    QMap<QString, QDBusMessage> monitorMsgMap;
    insertMessage(monitorMsgMap, "GetRecordSaveSwitch", rst << true);
    insertMessage(monitorMsgMap, "GetLimitModel", rst << USB_LIMIT_MODEL_ALL);
    insertMessage(monitorMsgMap, "GetUsbConnectionRecords", rst << "");
    insertMessage(monitorMsgMap, "GetWhiteList", rst << "");

    for (auto iter = monitorMsgMap.begin(); iter != monitorMsgMap.end(); iter++) {
        QString invokeName = iter.key();
        EXPECT_CALL(monitorNetFlowMockInter, Invoke(invokeName, _, _))
            .WillRepeatedly(Return(monitorMsgMap[invokeName]));
    }
    EXPECT_CALL(monitorNetFlowMockInter, EmitSignal(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(monitorNetFlowMockInter, Connect(_, _, _)).WillRepeatedly(Return(true));

    UsbConnectionWork work(&factory);
    UsbConnectionWidget widget(&work);
}
