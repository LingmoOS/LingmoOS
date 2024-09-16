// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncmodel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusInterface>
#include <QJsonArray>

SyncModel::SyncModel(QObject *parent)
    : QObject(parent)
    , m_syncIsValid(false)
    , m_enableSync(false)
    , m_activation(false)
    , m_lastSyncTime(0)
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
}

void SyncModel::setUserinfo(const QVariantMap &userinfo)
{
    if (m_userinfo == userinfo) return;

    m_userinfo = userinfo;

    Q_EMIT userInfoChanged(userinfo);
}

void SyncModel::setSyncState(const std::pair<qint32, QString> &syncState)
{
    if (m_syncState == syncState) return;

    m_syncState = syncState;

    Q_EMIT syncStateChanged(syncState);
}

bool SyncModel::isSyncStateValid(const std::pair<qint32, QString> &state)
{
    return state.first == 100 || state.first == 200 || state.first >= 1000;
}

bool SyncModel::isSyncFailed(const std::pair<qint32, QString> &state)
{
    // NOTE(justforlxz): Daemon definition is greater than 1000 is wrong
    return state.first >= 1000;
}

bool SyncModel::isSyncing(const std::pair<qint32, QString> &state)
{
    return state.first == 100;
}

bool SyncModel::isSyncSucceed(const std::pair<qint32, QString> &state)
{
    return state.first == 200;
}

void SyncModel::setLastSyncTime(const qlonglong &lastSyncTime)
{
    if (m_lastSyncTime == lastSyncTime) return;

    m_lastSyncTime = lastSyncTime;

    Q_EMIT lastSyncTimeChanged(lastSyncTime);
}

void SyncModel::setEnableSync(bool enableSync)
{
    if (m_enableSync == enableSync) return;

    m_enableSync = enableSync;

    Q_EMIT enableSyncChanged(enableSync);
}

QString SyncModel::getDeviceCode()
{
    QDBusInterface Interface("com.deepin.deepinid",
                             "/com/deepin/deepinid",
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::sessionBus());
    QDBusMessage reply = Interface.call("Get", "com.deepin.deepinid", "HardwareID");
    QList<QVariant> outArgs = reply.arguments();
    QString deviceCode = outArgs.at(0).value<QDBusVariant>().variant().toString();
    return deviceCode;
}

// com.deepin.sync.Daemon
std::list<std::pair<SyncType, QStringList>> SyncModel::moduleMap()
{
    return std::list<std::pair<SyncType, QStringList>>{
    { Network, { "network" } },
    { Sound, { "audio" } },
    { Mouse, { "peripherals" } },
    { Update, { "updater" } },
    { Dock, { "dock" } },
    { Launcher, { "launcher" } },
    { Wallpaper, { "background", "screensaver" } },
    { Theme, { "appearance" } },
    { Power, { "power" } },
};
}

void SyncModel::setModuleSyncState(SyncType type, bool state)
{
    m_moduleSyncState[type] = state;

    Q_EMIT moduleSyncStateChanged(std::pair<SyncType, bool>(type, state));
}

void SyncModel::setSyncIsValid(bool syncIsValid)
{
    if (m_syncIsValid == syncIsValid) return;

    m_syncIsValid = syncIsValid;

    Q_EMIT syncIsValidChanged(syncIsValid);
}

QString SyncModel::userDisplayName() const
{
    QString username = m_userinfo["Nickname"].toString();
    if (username.isEmpty()) {
        username = m_userinfo["Username"].toString();
    }
    return username;
}

void SyncModel::setActivation(bool value)
{
    if (m_activation == value)
        return;
    m_activation = value;
    Q_EMIT licenseStateChanged(value);
}

void SyncModel::setUUID(const QString &uuid)
{
    m_uuID = uuid;
    Q_EMIT userUUIDInfoChanged(uuid);
}

void SyncModel::setUOSID(const QString &uosid)
{
    m_uosID = uosid;
    Q_EMIT userUOSIDInfoChanged(uosid);
}

void SyncModel::setHostName(const QString &hostname)
{
    m_hostName = hostname;
    Q_EMIT userHostNameInfoChanged(hostname);
}

void SyncModel::addTrustDeviceList(const QJsonArray &devArray)
{
    QList<DeviceInfo> lstDevInfo;
    for (auto jsonValue: devArray)
    {
        QJsonObject obj = jsonValue.toObject();
        DeviceInfo devInfo;
        devInfo.devId = obj["id"].toString();
        devInfo.devCode = obj["deviceCode"].toString();
        devInfo.devType = (DeviceType)obj["deviceType"].toInt();
        devInfo.devName = obj["hostName"].toString();
        devInfo.osVersion = obj["osVersion"].toString();
        lstDevInfo << devInfo;
    }

    if(!lstDevInfo.isEmpty())
    {
        Q_EMIT addDeviceList(lstDevInfo);
    }
}

/**
 * @brief SyncModel::setBindLocalUBid
 * @param bindUBid 若为空 则未绑定或解绑
 */
void SyncModel::setBindLocalUBid(const QString &bindUBid)
{
    m_ubID = bindUBid;
    Q_EMIT userUnbindInfoChanged(bindUBid);
}

void SyncModel::setResetPasswdError(const QString &errorInfo)
{
    Q_EMIT resetPasswdError(errorInfo);
}

void SyncModel::setResetUserNameError(const QString &errorInfo)
{
    Q_EMIT resetUserNameError(errorInfo);
}

void SyncModel::addSyncItem(QList<Apps*> &appDate)
{
    // 查重
    m_switcherDumpDate.swap(appDate);
    Q_EMIT syncAppsChanged(m_switcherDumpDate);
}

void SyncModel::setUtcloudSwitcherState(const QString &itemKey, bool state)
{
    m_utcloudSyncState[itemKey] = state;
    for (auto *app: m_switcherDumpDate) {
        if(app->datekey() == itemKey) {
            app->setEnable(state);
            break;
        }
    }

    Q_EMIT utcloudSwitcherChange(itemKey, state);
}

void Apps::setName(const QString &name)
{
    if (m_name != name)
        m_name = name;
}

void Apps::setKey(const QString &Key)
{
    if (m_dateKey != Key)
        m_dateKey = Key;
}

void Apps::setEnable(bool enable)
{
    if (m_enable != enable)
        m_enable = enable;
}

void Apps::setShowState(bool state)
{
    if (m_showState != state)
        m_showState = state;
}

void Apps::setIconPath(const QString &path)
{
    if (m_iconPath != path)
        m_iconPath = path;
}
