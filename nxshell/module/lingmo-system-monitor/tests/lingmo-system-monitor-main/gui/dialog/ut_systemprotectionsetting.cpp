// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "dialog/systemprotectionsetting.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <DSettingsDialog>
#include <DSettingsOption>


/***************************************STUB begin*********************************************/
QDBusMessage stub_onMessgaeSetting_call()
{
    return QDBusMessage();
}
/***************************************STUB end**********************************************/


class UT_SystemProtectionSetting : public ::testing::Test
{
public:
    UT_SystemProtectionSetting() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new SystemProtectionSetting(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemProtectionSetting *m_tester;
};

TEST_F(UT_SystemProtectionSetting, initTest)
{

}

TEST_F(UT_SystemProtectionSetting, test_instance_01)
{
    EXPECT_TRUE(m_tester->instance());
}

TEST_F(UT_SystemProtectionSetting, test_createProtectionSwitchHandle_01)
{
    static DSettingsOption option;
    m_tester->createProtectionSwitchHandle(&option);
}

TEST_F(UT_SystemProtectionSetting, test_createAlarmUsgaeSettingHandle_01)
{
    static DSettingsOption option;
    m_tester->createAlarmUsgaeSettingHandle(&option);
}

TEST_F(UT_SystemProtectionSetting, test_createAlarmIntervalSettingHandle_01)
{
    static DSettingsOption option;
    m_tester->createAlarmIntervalSettingHandle(&option);
}

TEST_F(UT_SystemProtectionSetting, test_createSettingLinkButtonHandle_01)
{
    static DSettingsOption option;
    m_tester->createSettingLinkButtonHandle(&option);
}

TEST_F(UT_SystemProtectionSetting, test_getDSettingPointor_01)
{
    EXPECT_NE(m_tester->getDSettingPointor(), nullptr);
}

TEST_F(UT_SystemProtectionSetting, test_setLastValidAlarm_01)
{
    static DLineEdit lineEdit;
    static DSettingsOption option;
    m_tester->setLastValidAlarm(&lineEdit, &option, 10, 5, 1);
}

TEST_F(UT_SystemProtectionSetting, test_setLastValidAlarm_02)
{
    static DLineEdit lineEdit;
    static DSettingsOption option;
    m_tester->setLastValidAlarm(&lineEdit, &option, 10, 5, 6);
}

TEST_F(UT_SystemProtectionSetting, test_regularNumber_01)
{
    static DLineEdit lineEdit;
    m_tester->regularNumber(&lineEdit);
}

TEST_F(UT_SystemProtectionSetting, test_onMessgaeSetting_01)
{
    Stub stub;
    stub.set(ADDR(QDBusConnection, call), stub_onMessgaeSetting_call);
    m_tester->onMessgaeSetting("1");
}

TEST_F(UT_SystemProtectionSetting, test_onSettingItemChanged_01)
{
    m_tester->onSettingItemChanged("setting.systemprotection.alarm_switch", "on");
}

TEST_F(UT_SystemProtectionSetting, test_onSettingItemChanged_02)
{
    m_tester->onSettingItemChanged("setting.systemprotection.alarm_cpu_usage", "on");
}

TEST_F(UT_SystemProtectionSetting, test_onSettingItemChanged_03)
{
    m_tester->onSettingItemChanged("setting.systemprotection.alarm_memory_usage", "on");
}

TEST_F(UT_SystemProtectionSetting, test_onSettingItemChanged_04)
{
    m_tester->onSettingItemChanged("setting.systemprotection.alarm_interval", "on");
}

TEST_F(UT_SystemProtectionSetting, test_onSettingItemChanged_05)
{
    m_tester->onSettingItemChanged("setting.systemprotection.alarm_last_time", "on");
}

TEST_F(UT_SystemProtectionSetting, test_onUpdateNewBackend_01)
{
    m_tester->onUpdateNewBackend();
}

TEST_F(UT_SystemProtectionSetting, test_lineEditChanged_01)
{
    static DLineEdit lineEdit;
    static DSettingsOption option;

    m_tester->lineEditChanged(false, &lineEdit, &option, 100, 30);
}









