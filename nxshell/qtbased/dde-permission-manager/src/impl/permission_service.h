// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AD0CF546_BF40_4B81_864F_93F98EEE99AF
#define AD0CF546_BF40_4B81_864F_93F98EEE99AF

#include <QObject>
#include <QDBusContext>
#include <QVector>
#include <QJsonDocument>
#include <QJsonArray>

#include "../utils/dconfig_settings.h"
#include "../../dbus/types/permissions_info.h"
#include "permissionpolicy.h"

DCORE_USE_NAMESPACE

struct AppPermissionInfo {
    QVariantMap appPmInfo;
};

struct PermissionInfo {
    QVariantMap pmName;
    QMap<QString, QList<AppPermissionInfo>> appInfoMap;

    QByteArray toJson(const PermissionInfo &pmInfo) {
        QJsonArray appInfoArray;
        QString pmName = "";
        if (pmInfo.pmName.contains("permission")) {
            pmName = pmInfo.pmName["permission"].toString().simplified();
            for (auto appInfos : pmInfo.appInfoMap) {
                for (auto appInfo : appInfos) {
                    QJsonObject obj;
                    for (auto it = appInfo.appPmInfo.begin(); it != appInfo.appPmInfo.end(); ++it) {
                        obj.insert(it.key().simplified(), QJsonValue::fromVariant(it.value()));
                    }
                    appInfoArray.append(QJsonValue(obj));
                }
            }
        }

        QJsonObject obj1 = {
            {"permission", pmName},
            {"appInfo", appInfoArray}
        };

        return QJsonDocument(obj1).toJson();
    }
};

struct GroupPermissionInfo {
    QVariantMap group;
    QMap<QString, QList<PermissionInfo>> permissionInfo;

    QByteArray toJson(const GroupPermissionInfo &groupPmInfo) {
        QJsonArray groupPmInfoArray;
        QString groupName = "";
        if (groupPmInfo.group.contains("group")) {
            groupName = groupPmInfo.group["group"].toString();
            for (auto pmInfos : groupPmInfo.permissionInfo) {
                for (auto pmInfo : pmInfos) {
                    QJsonObject obj = QJsonDocument::fromJson(pmInfo.toJson(pmInfo)).object();
                    groupPmInfoArray.append(QJsonValue(obj));
                }
            }
        }

        QJsonObject obj1 = {
            {"group", groupName},
            {"permissionInfo", groupPmInfoArray}
        };

        return QJsonDocument(obj1).toJson();
    }
};

class PermissionService : public QObject, protected QDBusContext {
    Q_OBJECT
public:
    explicit PermissionService(QObject *parent = nullptr);
    ~PermissionService();

public Q_SLOTS: // METHODS
    QStringList PermissionList(const QString &permissionGroup);
    QString Request(const QString &appId, const QString &permissionGroup,const QString &permissionId);
    QString Request(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &title, const QString &description);
    QStringList Request(const QString &appId, const QString &permissionGroup, const QStringList &permissionId);
    void Reset(const QString &appId, const QString &permissionGroup);
    void Set(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &value);
    QStringList GetRegisterAppPermissionIds(const QString &appId, const QString &permissionGroup);
    void RegistAppPermissions(const QString &appId, const QString &permissionGroup, const PermissionsInfo &permissionsInfo);
    void ShowDisablePermissionDialog(const QString &appId, const QString &permissionGroup, const QString &permissionId);

    // 获取各权限 应用的权限信息，eq：相机：{浏览器：1, 终端: 1}
    QString GetPermissionInfo();
    bool SetPermissionInfo(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &value);
    bool GetPermissionEnable(const QString &permissionGroup, const QString &permissionId);
    bool SetPermissionEnable(const QString &permissionGroup, const QString &permissionId, const bool &enable);

Q_SIGNALS:
    void PermissionInfoChanged();
    void PermissionEnableChanged(const QString &permissionGroup, const QString &permissionId, const bool &enable);

private:
    QString parseAppPermissionKeyDconf(const QString& appPermissionDconfKey, const QString &appId, const QString &permissionGroup, const QString &permissionId, const PermissionPolicy& policy, const QString &title, const QString &description);
    AppPermissionInfo addAppPermissionInfo(const QString &appName, const QString &pmValue);
    QList<AppPermissionInfo> addPermissionInfo(const QString &appName, const QString &pmValue);
    PermissionInfo addPermissionInfo(const QString &pmName, const QString &appName, const QString &pmValue);
    GroupPermissionInfo addGroupPermissionInfo(const QString &group, const QString &pmName, const QString &appName, const QString &pmValue);
    void adjustPermissionInfo(const QString& appPermissionDconfKey);

private:
    QList<GroupPermissionInfo> m_groupPermissionInfo;
};

#endif /* AD0CF546_BF40_4B81_864F_93F98EEE99AF */
