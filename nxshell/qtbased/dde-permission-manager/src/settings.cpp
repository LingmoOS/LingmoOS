// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings.h"

#include <QException>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

struct SettingsPrivate {
    Settings* q_ptr;
    QString   configPath;
    QString   defaultConfigPath;
    explicit SettingsPrivate(Settings* settings) : q_ptr(settings)
    {
        const QString path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
        configPath         = QString("%1/%2/").arg(path, "applications/permission");
        defaultConfigPath = "/usr/share/permission/defaultapppermission";
    }

    ~SettingsPrivate() {}

    // 检查配置文件，如果不存在则复制默认的配置
    bool defaultConfigCheck(const QString& appId, const QString& permissionGroup) {
        const QString appConfig = QString("%1/%2/%3").arg(configPath, permissionGroup, appId);
        if (QFile::exists(appConfig)) {
            return true;
        }
        const QString appDefaultConfig = QString("%1/%2/%3").arg(defaultConfigPath, permissionGroup, appId);
        if (QFile::exists(appDefaultConfig)) {
            if (!QDir().mkpath(QString("%1/%2").arg(configPath, permissionGroup))) {
                qWarning() << "failed to create dir";
                return false;
            }
            if (!QFile::copy(appDefaultConfig, appConfig)) {
                qWarning() << "default config copy fail";
                return false;
            }
            return true;
        }
        return false;
    }

    bool saveSettings(const SettingData& data) {
        if (!QDir().mkpath(QString("%1/%2").arg(configPath, data.PermissionGroup))) {
            qWarning() << "failed to create dir";
            return false;
        }
        const QString appConfig = QString("%1/%2/%3").arg(configPath, data.PermissionGroup, data.AppId);
        QSettings settings(appConfig, QSettings::IniFormat);
        settings.beginGroup("permission");
        settings.setValue(data.PermissionId, data.Result);

        return settings.value(data.PermissionId).value<SettingData::AccessResult>() == data.Result;
    }

    SettingData::AccessResult result(const QString& appId, const QString& permissionGroup, const QString& permissionId)
    {
        const QString appConfig = QString("%1/%2/%3").arg(configPath, permissionGroup, appId);
        if (!QFile::exists(appConfig)) {
            throw QException();
        }

        QSettings settings(appConfig, QSettings::IniFormat);
        settings.beginGroup("permission");
        const QVariant value = settings.value(permissionId);
        if (!value.isValid()) {
            throw QException();
        }

        return value.value<SettingData::AccessResult>();
    }

    bool removeSettings(const QString& appId, const QString& permissionGroup)
    {
        const QString appConfig = QString("%1/%2/%3").arg(configPath, permissionGroup, appId);
        if (QFile::exists(appConfig)) {
            return QFile::remove(appConfig);
        }
        return true;
    }
};

Settings::Settings() : d_ptr(new SettingsPrivate(this)) {}
Settings::~Settings() {}

bool Settings::saveSettings(const SettingData& data)
{
    d_ptr->defaultConfigCheck(data.AppId, data.PermissionGroup);
    return d_ptr->saveSettings(data);
}

SettingData::AccessResult Settings::result(const QString& id, const QString& permissionGroup, const QString& permissionId)
{
    try {
        if (!d_ptr->defaultConfigCheck(id, permissionGroup)) {
            throw QException();
        }
        auto result = d_ptr->result(id, permissionGroup, permissionId);
        return result;
    }
    catch (QException &exp) {
        throw QException();
    }
}

bool Settings::removeSettings(const QString& appId, const QString& permissionGroup)
{
    return d_ptr->removeSettings(appId, permissionGroup);
}
