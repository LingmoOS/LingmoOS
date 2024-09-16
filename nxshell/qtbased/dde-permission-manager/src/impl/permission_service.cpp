// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "permission_service.h"
#include "permissionoption.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QException>

#include "settings.h"
#include "clientinterface.h"
#include "../utils/common.h"
#include "../utils/phase_pm_dconfig.h"

DCORE_USE_NAMESPACE

const QString polikitPermissionActionId = "org.deepin.dde.permission.modify-app-permissions";
const QString dbusErrorName = "org.deepin.dde.Permission1.";
const QString sessionPermissionsEnable = "Session_Permissions_Enable";
const QString systemPermissionsEnable  = "System_Permissions_Enable";

PermissionService::PermissionService(QObject *parent) : QObject(parent), QDBusContext()
{
    registerPermissionsInfoMetaType();
}

PermissionService::~PermissionService() {}

QStringList PermissionService::PermissionList(const QString &permissionGroup)
{
    return PermissionPolicy::getPolicyList(permissionGroup);
}

QString PermissionService::Request(const QString &appId, const QString &permissionGroup, const QString &permissionId)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        qWarning() << "Request policy invalid.";
        sendErrorReply(dbusErrorName + "PermissionPolicyError","Permission instance JSON file error.");
        return "";
    }

    return Request(appId, permissionGroup, permissionId, policy.title("requestPermission"), policy.description("requestPermission"));
}

QString PermissionService::Request(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &title, const QString &description)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        qWarning() << "Request policy invalid.";
        sendErrorReply(dbusErrorName + "PermissionPolicyError","Permission instance JSON file error.");
        return "";
    }

    if (!GetPermissionEnable(permissionGroup, permissionId)) {
        // 权限总开关关闭时，无论请求什么什么权限，直接返回禁止
        // 不需要弹框，返回禁止后，调用方会触发弹窗逻辑
        return QString::number(dconfDeny);
    }

    QString ret = "";
    if (policy.type() == "session") {
        ret = parseAppPermissionKeyDconf(sessionAppPermissionDconfKey, appId, permissionGroup, permissionId, policy, title, description);
    } else if (policy.type() == "system") {
        ret = parseAppPermissionKeyDconf(systemAppPermissionDconfKey, appId, permissionGroup, permissionId, policy, title, description);
    } else {
        sendErrorReply(dbusErrorName + "PermissionPolicyError","Permission instance JSON file error.");
        return "";
    }

    return ret;
}

QStringList PermissionService::Request(const QString &appId, const QString &permissionGroup, const QStringList &permissionId)
{
    // check valid before Request
    for (const QString &id : permissionId) {
        // TODO
        Q_UNUSED(id)
    }

    QStringList result;
    for (const QString &id : permissionId) {
        // read permission description
        result << Request(appId, permissionGroup, id);
    }

    return result;
}

void PermissionService::Reset(const QString &appId, const QString &permissionGroup)
{
    PhasePMDconfig::resetAppPermission(appId, permissionGroup);
    Q_EMIT PermissionInfoChanged();
}

void PermissionService::Set(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &value)
{
    // todo
    Q_UNUSED(appId);
    Q_UNUSED(permissionGroup);
    Q_UNUSED(permissionId);
    Q_UNUSED(value);
//    SettingData data{
//        appId,
//        permissionGroup,
//        permissionId,
//        value,
//    };
//    Settings settings;
//    if (PermissionOption::getInstance()->isNeedSavedByIndex(value)) {
//        settings.saveSettings(data);
//    }
}

void PermissionService::RegistAppPermissions(const QString &appId, const QString &permissionGroup, const PermissionsInfo &permissionsInfo)
{
    if (!PhasePMDconfig::setPermissionInfo(appId, permissionGroup, systemAppPermissionRegistKey, permissionsInfo)) {
        sendErrorReply(dbusErrorName + "RegistPermissionValueError","The registered permission value is incorrect");
    }
}

QStringList PermissionService::GetRegisterAppPermissionIds(const QString &appId, const QString &permissionGroup)
{
    return PhasePMDconfig::getPermissionList(appId, permissionGroup, systemAppPermissionRegistKey);
}

void PermissionService::ShowDisablePermissionDialog(const QString &appId, const QString &permissionGroup, const QString &permissionId)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        return;
    }
    const QStringList options = {"Cancel", "Go to Settings"};
    org::deepin::dde::Permission1::Client client("org.deepin.dde.Permission1.Client", "/org/deepin/dde/Permission1/Client", QDBusConnection::sessionBus());
    auto reply = client.ShowDisablePermissionDialog(policy.title("denyPrompt").arg(appId), policy.description("denyPrompt"), options);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "[DBus] [Warning] " << reply.error();
        return;
    }

    if (reply.value() == options[1]) {
        QString arg = QString("privacyAndSecurity/%1").arg(permissionId);
        QDBusMessage message = QDBusMessage::createMethodCall("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1", "ShowPage");
        message << arg;
        QDBusConnection::sessionBus().asyncCall(message);
    }

    return;
}

int showRequestDialog(const QString &appId, const PermissionPolicy& policy, const QString &title, const QString &description)
{
    org::deepin::dde::Permission1::Client client("org.deepin.dde.Permission1.Client", "/org/deepin/dde/Permission1/Client", QDBusConnection::sessionBus());
    auto                                 reply = client.Request(title.arg(appId), description, policy.prefer(), policy.options());
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "[DBus] [Warning] " << reply.error();
        return -1;
    }

    if (!policy.options().contains(reply.value())) {
        qWarning() << "the error button is pressed";
        return -1;
    }


    return PermissionOption::getInstance()->getId(reply.value());
}

QString PermissionService::parseAppPermissionKeyDconf(const QString& appPermissionDconfKey, const QString &appId, const QString &permissionGroup, const QString &permissionId, const PermissionPolicy& policy, const QString &title, const QString &description)
{
    const QString& registerMode = policy.registerMode();
    bool needShowDialog = false;
    bool allowSkipLimitCheck = false;
    int appPmDconfRet = PhasePMDconfig::getPermissionValue(appId, permissionGroup, appPermissionDconfKey, permissionId);
    int registPmDconfRet = PhasePMDconfig::getPermissionValue(appId, permissionGroup, systemAppPermissionRegistKey, permissionId);
    QString strAppPmDconfRet = QString::number(appPmDconfRet);
    QString strRegistPmDconfRet = QString::number(registPmDconfRet);

    if (appPmDconfRet < dconfValueInvalid || appPmDconfRet > dconfMax) {
        qWarning() << QString("get [%1] failed [%2].").arg(appPermissionDconfKey, appPmDconfRet);
        sendErrorReply(dbusErrorName + "RegistPermissionValueError","The registered permission value is incorrect");
        return "";
    }

    if (registPmDconfRet < dconfValueInvalid || appPmDconfRet > dconfMax) {
        qWarning() << QString("get [%1] failed [%2].").arg(appPermissionDconfKey, registPmDconfRet);
        sendErrorReply(dbusErrorName + "RegistPermissionValueError","The registered permission value is incorrect");
        return "";
    }

    if (appPmDconfRet == dconfKeyInvalid) {
        if (registPmDconfRet == dconfKeyInvalid) {
            needShowDialog = true;
        } else {
            QVariantMap mapTmp = PhasePMDconfig::getAppPermissionMap(appId, permissionGroup, systemAppPermissionRegistKey);
            PhasePMDconfig::setPermissionMap(appId, permissionGroup, appPermissionDconfKey, mapTmp);
            if (registPmDconfRet == dconfValueInvalid || registPmDconfRet == dconfNullValue) {
                needShowDialog = true;
                allowSkipLimitCheck = true;
            } else {
                return strRegistPmDconfRet;
            }
        }
    } else if (appPmDconfRet == dconfValueInvalid) {
        if (registPmDconfRet == dconfKeyInvalid) {
            needShowDialog = true;
        } else if (registPmDconfRet == dconfValueInvalid) {
            needShowDialog = true;
        } else if (registPmDconfRet == dconfNullValue) {
            needShowDialog = true;
            allowSkipLimitCheck = true;
        } else {
            PhasePMDconfig::setPermissionValue(appId, permissionGroup, appPermissionDconfKey, permissionId, QString::number(registPmDconfRet));
            return strRegistPmDconfRet;
        }
    } else if (appPmDconfRet == dconfNullValue) {
        needShowDialog = true;
        allowSkipLimitCheck = true;
    } else {
        // 直接返回，会出现普通用户访问系统级权限，当在system级别中找到后，直接返回 异常
        if (registerMode == "notlimit") {
            return strAppPmDconfRet;
        }
        needShowDialog = true;
    }

    if (needShowDialog) {
        if (registerMode != "notlimit" && !allowSkipLimitCheck) {
            qWarning() << QString("Currently in limit mode, the permissionId [%1] is not registered").arg(permissionId);
            sendErrorReply(dbusErrorName + "RegistModeError","This permission instance needs to be registered.");
            return "";
        }

        // 普通用户想要访问系统级权限，直接返回-1
        if (checkUserIsAdmin() == UserType::UserTypeStandard && policy.type() == "system") {
            // 此时需要写入应用配置文件， value为空
            PhasePMDconfig::setPermissionValue(appId, permissionGroup, appPermissionDconfKey, permissionId, "");
            Q_EMIT PermissionInfoChanged();
            sendErrorReply(dbusErrorName + "SystemLevelRestrictions","users want access to system-level permissions.");
            return "";
        }

        org::deepin::dde::Permission1::Client client("org.deepin.dde.Permission1.Client", "/org/deepin/dde/Permission1/Client", QDBusConnection::sessionBus());
        auto                                 reply = client.Request(title.arg(appId), description, policy.prefer(), policy.options());
        reply.waitForFinished();
        if (reply.isError()) {
            qWarning() << "[DBus] [Warning] " << reply.error();
            sendErrorReply(reply);
            return "";
        }

        if (!policy.options().contains(reply.value())) {
            qWarning() << "the error button is pressed";
            return "";
        }

        QString ret = QString::number(PermissionOption::getInstance()->getId(reply.value()));
        PhasePMDconfig::setPermissionValue(appId, permissionGroup, appPermissionDconfKey, permissionId, ret);
        Q_EMIT PermissionInfoChanged();

        // 用户通过request接口选择禁止权限，此时给调用方返回权限值2，调用方不去调起禁止权限框
        if (reply.value() == "deny") {
            qWarning() << "disable permission is selected";
            return "2";
        }
        return ret;
    }

    // 不可能走到此逻辑，特此加打印
    qWarning() << "error...";
    return "";
}

AppPermissionInfo PermissionService::addAppPermissionInfo(const QString &appName, const QString &pmValue)
{
    AppPermissionInfo appTmp;
    appTmp.appPmInfo.insert("appName", appName);
    appTmp.appPmInfo.insert("value", pmValue);

    return appTmp;
}

QList<AppPermissionInfo> PermissionService::addPermissionInfo(const QString &appName, const QString &pmValue)
{
    AppPermissionInfo tmpMap = addAppPermissionInfo(appName, pmValue);
    QList<AppPermissionInfo> tmpList;
    tmpList.append(tmpMap);

    return tmpList;
}

PermissionInfo PermissionService::addPermissionInfo(const QString &pmName, const QString &appName, const QString &pmValue)
{
    QList<AppPermissionInfo> tmpAppList = addPermissionInfo(appName, pmValue);
    PermissionInfo permissionInfo;
    permissionInfo.pmName.insert("permission", pmName);
    permissionInfo.appInfoMap.insert("appInfo", tmpAppList);

    return permissionInfo;
}

GroupPermissionInfo PermissionService::addGroupPermissionInfo(const QString &group, const QString &pmName, const QString &appName, const QString &pmValue)
{
    PermissionInfo permissionInfo = addPermissionInfo(pmName, appName, pmValue);
    QList<PermissionInfo> permissionInfoList;
    GroupPermissionInfo groupPermissionInfo;
    permissionInfoList.append(permissionInfo);
    groupPermissionInfo.group.insert("group", group);
    groupPermissionInfo.permissionInfo.insert("permissionInfo", permissionInfoList);

    return groupPermissionInfo;
}

void PermissionService::adjustPermissionInfo(const QString& appPermissionDconfKey)
{
    QString pmName = "";
    QString appName = "";
    QString group = "";
    QString pmValue = "";
    QVariantMap permissionMap = PhasePMDconfig::getPermissionMap(appPermissionDconfKey);
    for (auto it = permissionMap.begin(); it != permissionMap.end(); ++it) {
        QStringList keyList = it.key().split("+");
        if (keyList.isEmpty() || keyList.size() != 2) {
            qWarning() << "key phase failed: " << it.key();
            break;
        }

        appName = keyList[0];
        group = keyList[1];
        QVariantMap mapTmp = it.value().toMap();
        for (auto it1 = mapTmp.begin(); it1 != mapTmp.end(); ++it1) {
            pmName = it1.key();
            pmValue = it1.value().toString();
            if (m_groupPermissionInfo.isEmpty()) {
                // 没有group需要新增结构
                GroupPermissionInfo groupPermissionInfo = addGroupPermissionInfo(group, pmName, appName, pmValue);
                m_groupPermissionInfo.append(groupPermissionInfo);
                continue;
            }

            bool groupPmInfoAdded = false;
            for (int i = 0; i < m_groupPermissionInfo.size(); ++i) {
                auto it2 = &m_groupPermissionInfo[i];
                if (!it2->group.contains("group") || (it2->group.contains("group") && it2->group["group"] != group)) {
                    continue;
                } else if (!it2->permissionInfo.contains("permissionInfo")) {
                    continue;
                } else {
                    groupPmInfoAdded = true;
                    bool pmInfoAdded = false;
                    for (auto it3 = it2->permissionInfo["permissionInfo"].begin(); it3 != it2->permissionInfo["permissionInfo"].end(); ++it3) {
                        if (!it3->pmName.contains("permission")) {
                            // 将本次的添加
                            QList<AppPermissionInfo> tmpList = addPermissionInfo(appName, pmValue);
                            it3->pmName.insert("permission", pmName);
                            it3->appInfoMap.insert("appInfo", tmpList);
                            pmInfoAdded = true;
                            break;
                        }

                        if (it3->pmName["permission"] == pmName) {
                            pmInfoAdded = true;
                            if (it3->appInfoMap.isEmpty()) {
                                QList<AppPermissionInfo> tmpList = addPermissionInfo(appName, pmValue);
                                it3->appInfoMap.insert("appInfo", tmpList);
                                break;
                            } else if (it3->appInfoMap["appInfo"].isEmpty()){
                                AppPermissionInfo appTmp = addAppPermissionInfo(appName, pmValue);
                                it3->appInfoMap["appInfo"].append(appTmp);
                                break;
                            } else {
                                bool appInfoAdded = false;
                                for (auto it4 = it3->appInfoMap["appInfo"].begin(); it4 != it3->appInfoMap["appInfo"].end(); ++it4) {
                                    if (!it4->appPmInfo.contains("appName")) {
                                        it4->appPmInfo.insert("appName", appName);
                                        it4->appPmInfo.insert("value", pmValue);
                                        appInfoAdded = true;
                                        break;
                                    }

                                    if (it4->appPmInfo["appName"] == appName) {
                                        if (!it4->appPmInfo.contains("value")) {
                                            it4->appPmInfo.insert("value", pmValue);
                                        } else {
                                            it4->appPmInfo["value"] = pmValue;
                                        }
                                        appInfoAdded = true;
                                        break;
                                    }
                                }

                                if (!appInfoAdded) {
                                    AppPermissionInfo appTmp = addAppPermissionInfo(appName, pmValue);
                                    it3->appInfoMap["appInfo"].append(appTmp);
                                    break;
                                }
                            }
                        }
                    }

                    if (!pmInfoAdded) {
                        PermissionInfo permissionInfo = addPermissionInfo(pmName, appName, pmValue);
                        it2->permissionInfo["permissionInfo"].append(permissionInfo);
                        break;
                    }
                }
            }

            if (!groupPmInfoAdded) {
                GroupPermissionInfo groupPermissionInfo = addGroupPermissionInfo(group, pmName, appName, pmValue);
                m_groupPermissionInfo.append(groupPermissionInfo);
            }
        }
    }
}

QString PermissionService::GetPermissionInfo()
{
    adjustPermissionInfo(systemAppPermissionDconfKey);
    adjustPermissionInfo(sessionAppPermissionDconfKey);

    QJsonArray arr;
    for (int i = 0; i < m_groupPermissionInfo.size(); ++i) {
        QJsonDocument doc = QJsonDocument::fromJson(m_groupPermissionInfo[i].toJson(m_groupPermissionInfo[i]));
        arr.append(doc.object());
    }
    m_groupPermissionInfo.clear();

    QJsonDocument doc(arr);
    return doc.toJson(QJsonDocument::Compact);
}

bool PermissionService::SetPermissionInfo(const QString &appId, const QString &permissionGroup, const QString &permissionId, const QString &value)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        return false;
    }

    if (policy.type() == "session") {
        int pmDconfRet = PhasePMDconfig::getPermissionValue(appId, permissionGroup, sessionAppPermissionDconfKey, permissionId);
        // 此接口仅resist或者request之后的权限才能被设置
        if (pmDconfRet > dconfKeyInvalid && pmDconfRet < dconfMax) {
            Q_EMIT PermissionInfoChanged();
            return PhasePMDconfig::setPermissionValue(appId, permissionGroup, sessionAppPermissionDconfKey, permissionId, value);
        }

        return false;
    }

    if (policy.type() == "system") {
        if (checkUserIsAdmin() == UserType::UserTypeStandard) {
            // 普通用户想要访问系统级权限 弹出鉴权窗口
            const QDBusConnection &conn = connection();
            const QDBusMessage &msg = message();
            const int &pid = conn.interface()->servicePid(msg.service()).value();
            if (!checkAuth(polikitPermissionActionId, pid)) {
                qWarning() << "checkAuth failed id: " << polikitPermissionActionId;
                return false;
            }
        }
    } else {
        qWarning() << "permissionId error: " << permissionId;
        return false;
    }

    int pmDconfRet = PhasePMDconfig::getPermissionValue(appId, permissionGroup, systemAppPermissionDconfKey, permissionId);
    // 此接口仅resist或者request之后的权限才能被设置
    if (pmDconfRet > dconfKeyInvalid && pmDconfRet < dconfMax) {
        Q_EMIT PermissionInfoChanged();
        return PhasePMDconfig::setPermissionValue(appId, permissionGroup, systemAppPermissionDconfKey, permissionId, value);
    }

    return false;
}

bool PermissionService::GetPermissionEnable(const QString &permissionGroup, const QString &permissionId)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        return false;
    }

    QString pmEnableDconfKey = "";
    if (policy.type() == "session") {
        pmEnableDconfKey = sessionPermissionsEnable;
    } else if (policy.type() == "system") {
        pmEnableDconfKey = systemPermissionsEnable;
    } else {
        qWarning() << QString("policy type is invalid [%1].").arg(policy.type());
        return false;
    }

    return PhasePMDconfig::getPermissionEnable(permissionGroup, permissionId, pmEnableDconfKey);;
}

bool PermissionService::SetPermissionEnable(const QString &permissionGroup, const QString &permissionId, const bool &enable)
{
    PermissionPolicy policy(permissionGroup, permissionId);
    if (!policy.isValid()) {
        return false;
    }

    if (policy.type() == "session") {
        PhasePMDconfig::setPermissionEnable(permissionGroup, permissionId, enable, sessionPermissionsEnable);
        Q_EMIT PermissionEnableChanged(permissionGroup, permissionId, enable);
        return true;
    } else if (policy.type() == "system") {
        if (checkUserIsAdmin() == UserType::UserTypeAdmin) {
            PhasePMDconfig::setPermissionEnable(permissionGroup, permissionId, enable, systemPermissionsEnable);
            Q_EMIT PermissionEnableChanged(permissionGroup, permissionId, enable);
            return true;
        } else {
                const QDBusConnection &conn = connection();
                const QDBusMessage &msg = message();
                const int &pid = conn.interface()->servicePid(msg.service()).value();
                if (checkAuth(polikitPermissionActionId, pid)) {
                PhasePMDconfig::setPermissionEnable(permissionGroup, permissionId, enable, systemPermissionsEnable);
                Q_EMIT PermissionEnableChanged(permissionGroup, permissionId, enable);
                return true;
            }
        }
    }

    return false;
}
