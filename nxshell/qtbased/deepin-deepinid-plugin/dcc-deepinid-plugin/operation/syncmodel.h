// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCMODEL_H
#define SYNCMODEL_H

#include "utils.h"

#include <QObject>
#include <QVariantMap>
#include <utility>
#include <QJsonObject>

enum DeviceType:int
{
    DEVICETYPE_UNKNOWN = 0,
    DEVICETYPE_PC = 1,
    DEVICETYPE_TABLET = 2,
    DEVICETYPE_NOTEBOOK = 3,
};

struct DeviceInfo
{
    QString devId;
    QString devCode;
    DeviceType devType;
    QString osVersion;
    QString devName;
};

class Apps : public QObject {
    Q_OBJECT
public:
    explicit Apps(QObject *parent = nullptr) : QObject(parent) , m_name("") , m_enable(false), m_showState(false), m_iconPath(""){}
    ~Apps() {}

    inline QString name() const { return m_name; }
    void setName(const QString& name);

    inline QString datekey() const { return m_dateKey; }
    void setKey(const QString& Key);

    inline bool enable() const { return m_enable; }
    void setEnable(bool enable);

    inline bool state() const { return m_showState; }
    void setShowState(bool state);

    inline QString icon() const { return m_iconPath; }
    void setIconPath(const QString& path);
private:
    QString m_dateKey;
    QString m_name;
    bool m_enable;
    bool m_showState;
    QString m_iconPath;
};

class SyncModel : public QObject {
    Q_OBJECT
public:
    explicit SyncModel(QObject *parent = nullptr);

    inline const QVariantMap userinfo() const { return m_userinfo; }
    void setUserinfo(const QVariantMap &userinfo);

    inline std::pair<qint32, QString> syncState() const { return m_syncState; }
    void setSyncState(const std::pair<qint32, QString> &syncState);

    static bool isSyncStateValid(const std::pair<qint32, QString> &state);
    static bool isSyncSucceed(const std::pair<qint32, QString> &state);
    static bool isSyncFailed(const std::pair<qint32, QString> &state);
    static bool isSyncing(const std::pair<qint32, QString> &state);

    inline qlonglong lastSyncTime() const { return m_lastSyncTime; }
    void setLastSyncTime(const qlonglong &lastSyncTime);

    inline bool enableSync() const { return m_enableSync; }
    void setEnableSync(bool enableSync);

    QString getDeviceCode();

    static std::list<std::pair<SyncType, QStringList>> moduleMap();

    inline QMap<SyncType, bool> moduleSyncState() const { return m_moduleSyncState; }
    inline bool getModuleStateByType(SyncType type) {
        return m_moduleSyncState[type];
    }

    inline QMap<QString, bool> utcloudSyncState() const { return m_utcloudSyncState; }
    inline bool getUtcloudStateByType(QString type) {
        return m_utcloudSyncState[type];
    }

    void setModuleSyncState(SyncType type, bool state);

    inline bool syncIsValid() const { return m_syncIsValid; }
    void setSyncIsValid(bool syncIsValid);
    QString userDisplayName() const;

    inline bool getActivation() const { return m_activation; }
    void setActivation(bool value);

    inline QString getUUID() const  { return m_uuID; }
    void setUUID(const QString &uuid);

    inline QString getUOSID() const { return m_uosID; }
    void setUOSID(const QString &uosid);

    inline QString getHostName() const { return m_hostName; }
    void setHostName(const QString &hostname);

    void addTrustDeviceList(const QJsonArray &devArray);

    inline QString getUbID() const  { return m_ubID; }
    void setBindLocalUBid(const QString &bindUBid);

    void setResetPasswdError(const QString &errorInfo);
    void setResetUserNameError(const QString &errorInfo);

    QList<Apps*> syncItem() { return m_switcherDumpDate; }
    void addSyncItem(QList<Apps*> &appDate);

    // 同步状态
    void setUtcloudSwitcherState(const QString& itemKey, bool state);

Q_SIGNALS:
    void userInfoChanged(const QVariantMap &userInfo);
    void syncStateChanged(const std::pair<qint32, QString>& syncState);
    void lastSyncTimeChanged(const qlonglong lastSyncTime);
    void enableSyncChanged(bool enableSync);
    void moduleSyncStateChanged(std::pair<SyncType, bool> state);
    void syncIsValidChanged(bool syncIsValid);
    void licenseStateChanged(bool activation);
    void userUUIDInfoChanged(const QString& uuid);
    void userUOSIDInfoChanged(const QString& uuid);
    void userHostNameInfoChanged(const QString& uuid);
    void userUnbindInfoChanged(const QString& ubid);
    void resetPasswdError(const QString& errorInfo);
    void resetUserNameError(const QString& errorInfo);

    void switcherDumpChanged(const QString& switcherDump);
    void syncAppsChanged(QList<Apps*> &appsItem);

    void checkPasswd(const QString &passwd);
    void resetPasswd(const QString &oldpwd, const QString &newpwd);
    void utcloudSwitcherChange(const QString& itemKey, bool state);
    void addDeviceList(const QList<DeviceInfo> &devlist);
private:
    bool m_syncIsValid;
    bool m_enableSync;
    bool m_activation;
    QMap<SyncType, bool> m_moduleSyncState;
    QMap<QString, bool> m_utcloudSyncState;
    QVariantMap m_userinfo;
    std::pair<qint32, QString> m_syncState;
    qlonglong m_lastSyncTime;
    QString m_uosID;
    QString m_uuID;
    QString m_hostName;
    QString m_ubID;
    QString m_bindLocalUBid; // 绑定后的UBID(如果没有绑定，返回空字符串)
    QList<Apps *> m_switcherDumpDate;
};

#endif  // SYNCMODEL_H
