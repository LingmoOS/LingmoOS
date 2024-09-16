// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_homepagewidget.h"
#include "../src/window/modules/common/common.h"
#include "../src/window/modules/common/gsettingkey.h"
#include "global/mockdbusinvoker/mockinvokerfactory.h"
#include "global/mockdbusinvoker/mockinvoker.h"
#include "global/mockgsettings/mockgsettings.h"

using namespace testing;

HomepageWidgetTest::HomepageWidgetTest()
{
}

void HomepageWidgetTest::SetUp()
{
}

void HomepageWidgetTest::TearDown()
{
}

TEST_F(HomepageWidgetTest, Sample)
{
    MockDbusInvokerFactory factory;
    MockDbusInvokerImpl defenderDataMockInter;
    MockDbusInvokerImpl controlCenterMockInter;
    MockGSettingsImpl gSettingsMockInter;

    factory.setInvokerInteface("com.deepin.defender.datainterface", &defenderDataMockInter);
    factory.setInvokerInteface("com.deepin.dde.ControlCenter", &controlCenterMockInter);
    factory.setSettinsInteface(DEEPIN_DEFENDER_GSETTING_PATH, &gSettingsMockInter);

    // gsettings mock data
    QMap<QString, QVariant> gsettingsValueMap;
    gsettingsValueMap.insert(PWD_LIMIT_LEVEL, 0);
    gsettingsValueMap.insert(PWD_CHANGE_DEADLINE_TYPE, 0);

    for (auto iter = gsettingsValueMap.begin(); iter != gsettingsValueMap.end(); iter++) {
        QString key = iter.key();
        QVariant value = iter.value();
        EXPECT_CALL(gSettingsMockInter, GetValue(key)).WillRepeatedly(Return(value));
    }
    EXPECT_CALL(gSettingsMockInter, SetValue(_, _)).WillRepeatedly(Return());

    // datainteface mock返回处理
    EXPECT_CALL(defenderDataMockInter, Invoke(_, _, _)).WillRepeatedly(Return(QDBusMessage()));
    EXPECT_CALL(defenderDataMockInter, EmitSignal(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(defenderDataMockInter, Connect(_, _, _)).WillRepeatedly(Return(true));

    // controlCenterMockInter 返回处理
    EXPECT_CALL(controlCenterMockInter, Invoke(_, _, _)).WillRepeatedly(Return(QDBusMessage()));
    EXPECT_CALL(controlCenterMockInter, EmitSignal(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(controlCenterMockInter, Connect(_, _, _)).WillRepeatedly(Return(true));
}
