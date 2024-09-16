// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "notification/bubblemanager.h"
#undef private
#include "mockpersistence.h"
#include "mocknotifysetting.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;

class UT_BubbleManager : public testing::Test
{
public:
    void SetUp() override
    {
        persistence = new MockPersistence;
        persistencehelper = new PersistenceHelper;

        notifySetting = new MockNotifySetting;
        settingHelper = new NotifySettingHelper;

        EXPECT_CALL(*persistence, getAllNotify()).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::getAllNotify));

        EXPECT_CALL(*persistence, getAll()).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::getAll));

        EXPECT_CALL(*persistence, addOne(testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::addOne));

        EXPECT_CALL(*persistence, removeAll()).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::removeAll));

        EXPECT_CALL(*persistence, addAll(testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::addAll));

        EXPECT_CALL(*persistence, removeOne(testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::removeOne));

        EXPECT_CALL(*persistence, removeApp(testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::removeApp));

        EXPECT_CALL(*persistence, recordCount()).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::getRecordCount));

        EXPECT_CALL(*persistence, getById(testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::getById));

        EXPECT_CALL(*persistence, getFrom(testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(persistencehelper, &PersistenceHelper::getFrom));

        EXPECT_CALL(*notifySetting, initAllSettings()).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::initAllSettings));

        EXPECT_CALL(*notifySetting, setAppSetting(testing::_, testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::setAppSetting));
        EXPECT_CALL(*notifySetting, getAppSetting(testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getAppSetting));
        EXPECT_CALL(*notifySetting, setSystemSetting(testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::setSystemSetting));
        EXPECT_CALL(*notifySetting, getSystemSetting(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getSystemSetting));
        EXPECT_CALL(*notifySetting, getAppLists()).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getAppLists));
        EXPECT_CALL(*notifySetting, appAdded(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::appAdded));
        EXPECT_CALL(*notifySetting, appRemoved(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::appRemoved));

        EXPECT_CALL(*notifySetting, setAppSetting_v1(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::setAppSetting_v1));
        EXPECT_CALL(*notifySetting, getAppSettings_v1(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getAppSettings_v1));
        EXPECT_CALL(*notifySetting, setSystemSetting_v1(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::setSystemSetting_v1));
        EXPECT_CALL(*notifySetting, getSystemSetings_v1()).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getSystemSetings_v1));
        EXPECT_CALL(*notifySetting, setAllSetting_v1(testing::_)).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::setAllSetting_v1));
        EXPECT_CALL(*notifySetting, getAllSetings_v1()).
                    WillRepeatedly(testing::Invoke(settingHelper, &NotifySettingHelper::getAllSetings_v1));

        obj = new BubbleManager(persistence, notifySetting);
    }

    void TearDown() override
    {
        delete obj;
        delete persistence;
        delete persistencehelper;
        delete notifySetting;
        delete settingHelper;

        obj = nullptr;
        persistence = nullptr;
        persistencehelper = nullptr;
        notifySetting = nullptr;
        settingHelper = nullptr;
    }

public:
    BubbleManager *obj = nullptr;
    MockPersistence *persistence = nullptr;
    PersistenceHelper *persistencehelper = nullptr;
    MockNotifySetting *notifySetting = nullptr;
    NotifySettingHelper *settingHelper = nullptr;

};

TEST_F(UT_BubbleManager, coverageTest)
{
    obj->CloseNotification(0);
    obj->GetCapabilities();

    QString name;
    QString vender;
    QString version;
    QString var = obj->GetServerInformation(name, vender, version);

    obj->GetAllRecords();
    obj->GetRecordById("0");
    obj->GetRecordsFromId(1, "0");
    obj->RemoveRecord("0");
    obj->ClearRecords();
    obj->Toggle();
    obj->recordCount();
    obj->GetAppList();
    obj->GetAppInfo("deepin-editor", 1);
    obj->GetSystemInfo(1);
    obj->SetAppInfo("deepin-editor", 0, QDBusVariant(true));
    obj->SetSystemInfo(0, QDBusVariant(true));
    obj->getAllSetting();
    obj->setAllSetting("");
    obj->getAppSetting("");
    obj->getSystemSetting();
    obj->setSystemSetting("");
    obj->getLastStableRect(0);
    obj->geometryChanged();
}

TEST_F(UT_BubbleManager, NotifyTest)
{
    obj->Notify("deepin-editor", 1, "", "", "", QStringList(), QVariantMap(), 1);
    obj->SetSystemInfo(3, QDBusVariant("00:00"));
    obj->SetSystemInfo(4, QDBusVariant("00:00"));
    obj->Notify("dde-control-center", 1, "", "", "", QStringList(), QVariantMap(), 1);
    obj->Notify("deepin-editor", 1, "", "", "", QStringList(), QVariantMap(), 1);
}
