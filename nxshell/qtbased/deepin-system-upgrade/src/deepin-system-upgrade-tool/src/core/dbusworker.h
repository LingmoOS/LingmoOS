// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>

#include "constants.h"

struct SourceInfo {
    SourceInfo(const QString a = "", const QString m = "", const bool c = false)
    {
        addr = a;
        method = m;
        checkImage = c;
    }

    QString addr;
    QString method;
    bool checkImage;
};
Q_DECLARE_METATYPE(SourceInfo)

struct MigrateResult {
    int code;
    QString description;
};
Q_DECLARE_METATYPE(MigrateResult)

const SourceInfo brokenSourceInfo(QString(""), QString(""), false);

QDBusArgument &operator<<(QDBusArgument &a, const SourceInfo &i);
const QDBusArgument &operator>>(const QDBusArgument &a, SourceInfo &i);
QDBusArgument &operator<<(QDBusArgument &a, const MigrateResult &result);
const QDBusArgument &operator>>(const QDBusArgument &a, MigrateResult &result);

// Singleton
class DBusWorker : public QObject
{
    Q_OBJECT
public:
    bool m_isBackupApps;

    static DBusWorker* getInstance(QObject *parent = nullptr);
    bool IsOnBattery();
    const QString GetISOVersion();
    const QString GetDistroVer();
    const QString GetDistroID();
    const QString GetVersion();
    void initConnections();
    const QMap<QString, QString>& getCheckResultMap() const {
        return m_checkResultMap;
    }
    const SourceInfo getSource();
    void SendNotification(const QString title, const QString msg);
    void SetPlymouthTheme(QString theme);
    void RestorePlymouthTheme();
    void CancelBackupApp(bool cancel);
    UpgradeStage getUpgradeStage() {
        return m_upgradeStage;
    }
    void MigratePackages();
    void SetMigrateAppsList(const QStringList &apps);
    void StopUpgrade();
    void CheckISO(const QString &isoPath);

private:
    explicit DBusWorker(QObject *parent = nullptr);
    bool GetIsBackupApps();
    static DBusWorker *m_instance;

    bool m_isCheckCanceled;
    UpgradeStage m_upgradeStage = UpgradeStage::INIT;

    QMap<QString, QString> m_checkResultMap;
    QDBusInterface *m_lcsInter;
    QDBusInterface *m_pwrstatInter;
    QDBusInterface *m_checkerInter;
    QDBusInterface *m_configInter;
    QDBusInterface *m_appInter;
    QDBusInterface *m_versionInter;
    QDBusInterface *m_legacySysinfoInter;
    QDBusInterface *m_sysinfoInter;
    QDBusInterface *m_notifyInter;
    QDBusInterface *m_systemupgrade1Inter;
    QDBusInterface *m_isoInter;

Q_SIGNALS:
    void error(QString errorTitle, QString errorLog);
    void EvaluateSoftwareError(QString errorLog);
    void MigrateDone();
    void MigrateError(int type, QString msg);
    void GetMigrateListDone();

    void Assess(QString isoPath);
    void CheckResult(bool passed);
    void StartUpgradeCheck();
    void StartUpgradePreparation(QString isoPath);
    void StartBackup();
    void StartUpgrade();
    void StartRestoration();

    void cancelUpgradeCheck();
    void AppCheckProgressUpdate(int progress);
    void CheckProgressUpdate(qint64 progress);
    void UpgradePreparationUpdate(qint64 progress);
    void BackupProgressUpdate(qint64 progress);
    void RestorationProgressUpdate(qint64 progress);
    void AppsAvailable(const QMap<QString, QStringList> appSet);
    void MigrateStatus(const QString pkg, int status);

    void SetPlymouthThemeDone();

private Q_SLOTS:
    void onAssess(QString isoPath);
    void onStartUpgradeCheck();
    void onStartUpgradePreparation(QString isoPath);
    void onStartBackup();
    void onStartUpgrade();
    void onStartRestoration();
    void onAsyncCallFinished(QDBusPendingCallWatcher *watcher);

    void onAppsAvailable(const QDBusMessage &msg);
    void onCheckUpdate(const QDBusMessage &msg);
    void onCancelUpgradeCheck();
    void onVersionProgressUpdate(qint64 p);
};
