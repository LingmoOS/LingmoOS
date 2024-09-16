/*
 * Copyright (C) 2022 ~ 2023 Deepin Technology Co., Ltd.
 *
 * Author:     weizhixiang <weizhixiang@uniontech.com>
 *
 * Maintainer: weizhixiang <weizhixiang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "phase_pm_dconfig.h"

#include <QSharedPointer>
#include <QDebug>

const QString pmAppId = "dde-permission-manager";
const QString appPermissionDconfJson = "org.deepin.dde.permission.dconfig1";

const QString systemAppPermissionRegistKey = "System_App_Permissions_Regist_Info";
const QString sessionAppPermissionDconfKey = "Session_App_Permissions_List";
const QString systemAppPermissionDconfKey  = "System_App_Permissions_List";

PhasePMDconfig::PhasePMDconfig()
{
}

PhasePMDconfig::~PhasePMDconfig()
{
}

// 获取应用的权限列表
QStringList PhasePMDconfig::getPermissionList(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, appPermissionDconfKey);
    if (!appPermissionsMap.isEmpty()) {
        QStringList appPmList;
        for (auto it = appPermissionsMap.begin(); it != appPermissionsMap.end(); ++it) {
            appPmList += it.key();
        }
        return appPmList;
    }

    return {};
}

// 获取应用对应权限的值
int PhasePMDconfig::getPermissionValue(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey, const QString &permissionId)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, appPermissionDconfKey);
    if (!appPermissionsMap.isEmpty()) {
        if (appPermissionsMap.contains(permissionId)) {
            if (appPermissionsMap[permissionId].toString() == "") {
                return dconfNullValue;
            }

            bool toIntSuccess = false;
            int ret = appPermissionsMap[permissionId].toString().toInt(&toIntSuccess);
            return (toIntSuccess) ? ret : dconfInvalidFormat;
        } else {
            // dconf中该权限id的key存在，但是此权限id不存在
            qWarning() << QString("The [%1] key exists in the DCONF, but the permissionID [%2] does not in key.").arg(appPermissionDconfKey, permissionId);
            return dconfValueInvalid;
        }
    } else {
        // dconf中该权限id的key不存在
        qWarning() << QString("The [%1] key[%2] exists does not in the DCONF.").arg(appPermissionDconfKey, appPermissionsKey);
        return dconfKeyInvalid;
    }
}

// 获取应用的权限列表以及权限的值
QVariantMap PhasePMDconfig::getAppPermissionMap(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey)
{
    QMap<QString, QVariant> appPermissionsMap = PhasePMDconfig::getPermissionMap(appPermissionDconfKey);
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);

    if (appPermissionsMap.isEmpty()) {
        return {};
    } else {
        return appPermissionsMap[appPermissionsKey].toMap();
    }
}

QVariantMap PhasePMDconfig::getPermissionMap(const QString& appPermissionDconfKey)
{
    QVariant result = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, "");
    QMap<QString, QVariant> appPermissionsMap = result.toMap();

    return appPermissionsMap;
}

// 设置应用对应权限的值
bool PhasePMDconfig::setPermissionValue(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey, const QString &permissionId, const QString &permissionValue)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, appPermissionDconfKey);

    if (appPermissionsMap.isEmpty()) {
        // 将QMap<QString,int>转为QVaraintMap
        appPermissionsMap.insert(permissionId, permissionValue);
    } else {
        if (appPermissionsMap.contains(permissionId)) {
            appPermissionsMap[permissionId] = permissionValue;
        } else {
            appPermissionsMap.insert(permissionId, permissionValue);
        }
    }

    QVariantMap toDoWriteMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, "").toMap();
    toDoWriteMap.insert(appPermissionsKey, appPermissionsMap);
    DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, toDoWriteMap);

    return true;
}

// 设置应用的权限列表以及权限的值
bool PhasePMDconfig::setPermissionInfo(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey, const PermissionsInfo &permissionsInfo)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, appPermissionDconfKey);

    appPermissionsMap.clear();
    // 将QMap<QString,Qstring>转为QVaraintMap
    bool toIntSuccess = false;
    for (auto it = permissionsInfo.begin(); it != permissionsInfo.end(); ++it) {
        int ret = it.value().toInt(&toIntSuccess);
        if (!toIntSuccess || ret < dconfAllow || ret > dconfDeny) {
            return false;
        }
        appPermissionsMap.insert(it.key(), it.value());
    }

    QVariantMap toBeWrittenMap;
    toBeWrittenMap.insert(appPermissionsKey, appPermissionsMap);
    DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, toBeWrittenMap);

    return true;
}

void PhasePMDconfig::setPermissionMap(const QString &appId, const QString &permissionGroup, const QString& appPermissionDconfKey, const QVariantMap& dataMap)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, appPermissionDconfKey);
    if (!appPermissionsMap.isEmpty()) {
        for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
            if (appPermissionsMap.contains(it.key())) {
                appPermissionsMap[it.key()] = it.value();
            } else {
                appPermissionsMap.insert(it.key(), it.value());
            }
        }
    } else {
        appPermissionsMap = dataMap;
    }

    QVariantMap toBeWrittenMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, "").toMap();
    toBeWrittenMap.insert(appPermissionsKey, appPermissionsMap);
    DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, appPermissionDconfKey, toBeWrittenMap);
}

void PhasePMDconfig::resetAppPermission(const QString &appId, const QString &permissionGroup)
{
    QString appPermissionsKey = generatePermissionsKey(appId, permissionGroup);
    {
        QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, systemAppPermissionRegistKey);
        if (!appPermissionsMap.isEmpty()) {
            appPermissionsMap = {};
            QVariantMap toBeWrittenMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, systemAppPermissionRegistKey, "").toMap();
            toBeWrittenMap.insert(appPermissionsKey, appPermissionsMap);
            DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, systemAppPermissionRegistKey, toBeWrittenMap);
        }
    }

    {
        QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, systemAppPermissionDconfKey);
        if (!appPermissionsMap.isEmpty()) {
            appPermissionsMap = {};
            QVariantMap toBeWrittenMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, systemAppPermissionDconfKey, "").toMap();
            toBeWrittenMap.insert(appPermissionsKey, appPermissionsMap);
            DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, systemAppPermissionDconfKey, toBeWrittenMap);
        }
    }

    {
        QVariantMap appPermissionsMap = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, sessionAppPermissionDconfKey);
        if (!appPermissionsMap.isEmpty()) {
            appPermissionsMap = {};
            QVariantMap toBeWrittenMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, sessionAppPermissionDconfKey, "").toMap();
            toBeWrittenMap.insert(appPermissionsKey, appPermissionsMap);
            DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, sessionAppPermissionDconfKey, toBeWrittenMap);
        }
    }
}

bool PhasePMDconfig::getPermissionEnable(const QString &permissionGroup, const QString &permissionId, const QString& pmEnableDconfKey)
{
    QString pmEnableKey = generatePermissionsKey(permissionGroup, permissionId);
    QVariantMap pmEnableMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, pmEnableDconfKey, "").toMap();

    if (pmEnableMap.contains(pmEnableKey)) {
        return pmEnableMap[pmEnableKey].toBool();
    }

    // 默认值为开启
    qWarning() << QString("[%1] [%2] is not applied for in [%3]. The default value is returned.").arg(pmEnableDconfKey, permissionGroup, permissionId);
    return true;
}

void PhasePMDconfig::setPermissionEnable(const QString &permissionGroup, const QString &permissionId, const bool &enable, const QString& pmEnableDconfKey)
{
    QString pmEnableKey = generatePermissionsKey(permissionGroup, permissionId);
    QVariantMap pmEnableMap = DconfigSettings::ConfigValue(pmAppId, appPermissionDconfJson, pmEnableDconfKey, "").toMap();

    if (pmEnableMap.contains(pmEnableKey)) {
        pmEnableMap[pmEnableKey] = enable;
    } else {
        pmEnableMap.insert(pmEnableKey, enable);
    }

    DconfigSettings::ConfigSaveValue(pmAppId, appPermissionDconfJson, pmEnableDconfKey, pmEnableMap);
}
