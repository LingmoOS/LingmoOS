// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCKNOTIFYSETTING_H
#define MOCKNOTIFYSETTING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "notification/notifysettings.h"

class MockNotifySetting : public AbstractNotifySetting
{
public:
    MOCK_METHOD0(initAllSettings, void());
    MOCK_METHOD3(setAppSetting, void(const QString &, const AppConfigurationItem &, const QVariant &));
    MOCK_METHOD2(getAppSetting,  QVariant(const QString &, const AppConfigurationItem &));
    MOCK_METHOD2(setSystemSetting,  void(const SystemConfigurationItem &, const QVariant &));
    MOCK_METHOD1(getSystemSetting,  QVariant(const SystemConfigurationItem &));
    MOCK_METHOD0(getAppLists, QStringList());
    MOCK_METHOD1(appAdded, void(const LauncherItemInfo &));
    MOCK_METHOD1(appRemoved, void(const QString &));
    MOCK_METHOD1(setAppSetting_v1, void(QString));
    MOCK_METHOD1(getAppSettings_v1, QString(const QString &));
    MOCK_METHOD1(setSystemSetting_v1, void(const QString));
    MOCK_METHOD0(getSystemSetings_v1, QString());
    MOCK_METHOD1(setAllSetting_v1, void(QString));
    MOCK_METHOD0(getAllSetings_v1, QString());
};

// 虚拟数据类
class NotifySettingHelper
{
public:
    NotifySettingHelper();
    void initAllSettings();
    void setAppSetting(const QString &id,
                       const AbstractNotifySetting::AppConfigurationItem &item,
                       const QVariant &var);
    QVariant getAppSetting(const QString &id,
                           const AbstractNotifySetting::AppConfigurationItem &item) const;
    void setSystemSetting(const AbstractNotifySetting::SystemConfigurationItem &item,
                          const QVariant &var);
    QVariant getSystemSetting(const AbstractNotifySetting::SystemConfigurationItem &item) const;
    QStringList getAppLists();
    void appAdded(const LauncherItemInfo &info);
    void appRemoved(const QString &id);

    // 兼容之前版本的接口，后期移除
    void setAppSetting_v1(QString settings);
    QString getAppSettings_v1(const QString &id);
    void setSystemSetting_v1(QString settings);
    QString getSystemSetings_v1();
    void setAllSetting_v1(QString settings);
    QString getAllSetings_v1();

private:
    QMap<QString, QVariant> m_systemSetting;
    QMap<QString, QMap<QString, QVariant>> m_appSetting;
};

#endif // MOCKNOTIFYSETTING_H
