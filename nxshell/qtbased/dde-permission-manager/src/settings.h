// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef E19812E8_52FA_47ED_9D70_A707CE2E25E2
#define E19812E8_52FA_47ED_9D70_A707CE2E25E2

#include <QString>
#include <QDateTime>
#include <memory>

struct SettingData {
    typedef int AccessResult;

    QString AppId;
    QString PermissionGroup;
    QString PermissionId;
    AccessResult Result;
    QDateTime Modified = QDateTime::currentDateTimeUtc();
};

class SettingsPrivate;
class Settings {
public:
    Settings();
    ~Settings();

    bool saveSettings(const SettingData& data);
    SettingData::AccessResult result(const QString& appId, const QString& permissionGroup, const QString& permissionId);
    bool removeSettings(const QString& appId, const QString& permissionGroup);

private:
    std::unique_ptr<SettingsPrivate> d_ptr;
};

#endif /* E19812E8_52FA_47ED_9D70_A707CE2E25E2 */
