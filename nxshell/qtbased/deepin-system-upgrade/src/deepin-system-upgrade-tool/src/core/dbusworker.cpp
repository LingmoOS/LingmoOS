// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusPendingCall>
#include <QStringList>

#include "dbusworker.h"

QDBusArgument &operator<<(QDBusArgument &a, const SourceInfo &i)
{
    a.beginStructure();
    a << i.addr << i.method << i.checkImage;
    a.endStructure();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a, SourceInfo &i)
{
    a.beginStructure();
    a >> i.addr >> i.method >> i.checkImage;
    a.endStructure();
    return a;
}

QDBusArgument &operator<<(QDBusArgument &a, const MigrateResult &result)
{
    a.beginStructure();
    a << result.code << result.description;
    a.endStructure();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a, MigrateResult &result)
{
    a.beginStructure();
    a >> result.code >> result.description;
    a.endStructure();
    return a;
}

DBusWorker* DBusWorker::m_instance = nullptr;

DBusWorker* DBusWorker::getInstance(QObject *parent)
{
    return m_instance = m_instance ? m_instance : new DBusWorker(parent);
}

DBusWorker::DBusWorker(QObject *parent)
    : QObject(parent)
    , m_isBackupApps(true)
    , m_isCheckCanceled(false)
    , m_lcsInter(new QDBusInterface(
                   "com.deepin.license",
                   "/com/deepin/license/Info",
                   "com.deepin.license.Info",
                   QDBusConnection::systemBus(),
                   this
               ))
    , m_pwrstatInter(new QDBusInterface(
                       "org.freedesktop.UPower",
                       "/org/freedesktop/UPower",
                       "org.freedesktop.UPower",
                       QDBusConnection::systemBus(),
                       this
                   ))
    , m_checkerInter(new QDBusInterface(
                       "org.deepin.SystemUpgrade1",
                       "/org/deepin/SystemUpgrade1/Checker",
                       "org.deepin.SystemUpgrade1.Checker",
                       QDBusConnection::systemBus(),
                       this
                   ))
    , m_configInter(new QDBusInterface(
                      "org.deepin.SystemUpgrade1",
                      "/org/deepin/SystemUpgrade1/ConfigManager",
                      "org.deepin.SystemUpgrade1.ConfigManager",
                      QDBusConnection::systemBus(),
                      this
                  ))
    , m_appInter(new QDBusInterface(
                   "org.deepin.SystemUpgrade1",
                   "/org/deepin/SystemUpgrade1/AppManager",
                   "org.deepin.SystemUpgrade1.AppManager",
                   QDBusConnection::systemBus(),
                   this
               ))
    , m_versionInter(new QDBusInterface(
                       "org.deepin.SystemUpgrade1",
                       "/org/deepin/SystemUpgrade1/VersionManager",
                       "org.deepin.SystemUpgrade1.VersionManager",
                       QDBusConnection::systemBus(),
                       this
                   ))
    , m_legacySysinfoInter(new QDBusInterface(
                       "com.deepin.daemon.SystemInfo",
                       "/com/deepin/daemon/SystemInfo",
                       "com.deepin.daemon.SystemInfo",
                       QDBusConnection::sessionBus(),
                       this
                   ))
    , m_sysinfoInter(new QDBusInterface(
                       "org.deepin.dde.SystemInfo1",
                       "/org/deepin/dde/SystemInfo1",
                       "org.deepin.dde.SystemInfo1",
                       QDBusConnection::sessionBus(),
                       this
                   ))
    , m_notifyInter(new QDBusInterface(
                      "org.deepin.dde.Notification1",
                      "/org/deepin/dde/Notification1",
                      "org.deepin.dde.Notification1",
                      QDBusConnection::sessionBus(),
                      this
                  ))
    , m_systemupgrade1Inter(new QDBusInterface(
                           "org.deepin.SystemUpgrade1",
                           "/org/deepin/SystemUpgrade1",
                           "org.deepin.SystemUpgrade1",
                           QDBusConnection::systemBus(),
                           this
                       ))
    , m_isoInter(new QDBusInterface(
                       "org.deepin.SystemUpgrade1",
                       "/org/deepin/SystemUpgrade1/ISOManager",
                       "org.deepin.SystemUpgrade1.ISOManager",
                       QDBusConnection::systemBus(),
                       this
                   ))
{
    qDBusRegisterMetaType<SourceInfo>();
    qDBusRegisterMetaType<MigrateResult>();
    initConnections();
    m_isBackupApps = GetIsBackupApps();
    CancelBackupApp(!m_isBackupApps);
}

void DBusWorker::initConnections()
{
    QDBusConnection::systemBus().connect(
        m_checkerInter->service(),
        m_checkerInter->path(),
        "org.deepin.SystemUpgrade1.Checker",
        "CheckResult",
        "a{ss}t",
        this,
        SLOT(onCheckUpdate(QDBusMessage))
    );
    QDBusConnection::systemBus().connect(
        m_appInter->service(),
        m_appInter->path(),
        m_appInter->interface(),
        "AppsAvailable",
        "a{sas}",
        this,
        SLOT(onAppsAvailable(QDBusMessage))
    );

    connect(m_appInter, SIGNAL(ProgressValue(int)), this, SIGNAL(AppCheckProgressUpdate(int)));
    connect(m_checkerInter, SIGNAL(CheckProgressValue(qint64)), this, SIGNAL(CheckProgressUpdate(qint64)));
    connect(m_versionInter, SIGNAL(ProgressValue(qint64)), this, SLOT(onVersionProgressUpdate(qint64)));
    connect(this, SIGNAL(StartUpgrade()), this, SLOT(onStartUpgrade()));
    connect(this, SIGNAL(StartUpgradeCheck()), this, SLOT(onStartUpgradeCheck()));
    connect(this, SIGNAL(StartUpgradePreparation(QString)), this, SLOT(onStartUpgradePreparation(QString)));
    connect(this, SIGNAL(StartBackup()), this, SLOT(onStartBackup()));
    connect(this, SIGNAL(StartRestoration()), this, SLOT(onStartRestoration()));
    connect(this, SIGNAL(cancelUpgradeCheck()), this, SLOT(onCancelUpgradeCheck()));
    connect(this, SIGNAL(Assess(QString)), this, SLOT(onAssess(QString)));
    connect(m_appInter, SIGNAL(MigrateStatus(const QString, int)), this, SIGNAL(MigrateStatus(const QString, int)));
//    connect(this, &DBusWorker::SetPlymouthThemeDone, this, [this] { m_versionInter->call("StartSystemUpgrade"); });
    connect(m_isoInter, SIGNAL(CheckResult(bool)), this, SIGNAL(CheckResult(bool)));
}

const SourceInfo DBusWorker::getSource()
{
    QDBusMessage msg = m_configInter->call("GetSource");
    QVariant v = msg.arguments().at(0);
    QDBusArgument a = v.value<QDBusArgument>();

    SourceInfo info;
    a.beginArray();
    a >> info;
    a.endArray();

    return info;
}

bool DBusWorker::IsOnBattery()
{
    return m_pwrstatInter->property("OnBattery").toBool();
}

void DBusWorker::onAssess(QString isoPath)
{
    m_upgradeStage = UpgradeStage::EVALUATE_SOFTWARE;
    QDBusPendingCall pcall = m_appInter->asyncCall("Assess", isoPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::onStartUpgradeCheck()
{
    this->m_isCheckCanceled = false;
    m_checkerInter->setTimeout(kQDBusAsyncCallTimeout);
    m_appInter->setTimeout(kQDBusAsyncCallTimeout);
    m_checkerInter->asyncCall("Perform", 0);
}

void DBusWorker::onStartUpgradePreparation(QString isoPath)
{
    m_upgradeStage = UpgradeStage::PREPARATION;
    m_versionInter->setTimeout(kQDBusAsyncCallTimeout);
    QDBusPendingCall pcall = m_versionInter->asyncCall("PrepareForUpgrade", isoPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::onStartBackup()
{
    m_upgradeStage = UpgradeStage::BACKUP;
    m_versionInter->setTimeout(kQDBusAsyncCallTimeout);

    QDBusPendingCall pcall = m_versionInter->asyncCall("BackupSystem");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::onStartUpgrade()
{
//    SetPlymouthTheme("deepin-upgrade");
    m_versionInter->call("StartSystemUpgrade");
}

void DBusWorker::onStartRestoration()
{
    m_upgradeStage = UpgradeStage::RESTORATION;
    m_versionInter->setTimeout(kQDBusAsyncCallTimeout);

    QDBusPendingCall pcall = m_versionInter->asyncCall("RecoverSystem");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::onAppsAvailable(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    qDebug() << "apps available args count:" << arguments.count();
    if (1 != arguments.count())
        return;

    QMap<QString, QStringList> resultMap = qdbus_cast<QMap<QString, QStringList>>(arguments.at(0).value<QDBusArgument>());

    emit AppsAvailable(resultMap);
}

void DBusWorker::onCheckUpdate(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (2 != arguments.count())
        return;

    QMap<QString, QString> resultMap = qdbus_cast<QMap<QString, QString> >(arguments.at(0).value<QDBusArgument>());

    for(const auto &iter: resultMap.keys()) {
        m_checkResultMap.insert(iter, resultMap[iter]);
    }
}

void DBusWorker::onCancelUpgradeCheck()
{
    m_isCheckCanceled = true;
}

void DBusWorker::onVersionProgressUpdate(qint64 p)
{
    switch (m_upgradeStage)
    {
    case UpgradeStage::BACKUP:
        emit BackupProgressUpdate(p);
        break;
    case UpgradeStage::PREPARATION:
        emit UpgradePreparationUpdate(p);
        break;
    case UpgradeStage::RESTORATION:
        emit RestorationProgressUpdate(p);
        break;
    default:
        break;
    }
}

void DBusWorker::onAsyncCallFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError())
    {
        switch (m_upgradeStage)
        {
        case UpgradeStage::INIT:
            // Ignore errors that occurs out of upgrade stages
            break;
        case UpgradeStage::BACKUP:
            emit error(tr("Backup failed"), watcher->error().message());
            break;

        case UpgradeStage::PREPARATION:
            emit error(tr("Upgrade failed"), watcher->error().message());
            break;

        case UpgradeStage::RESTORATION:
            emit error(tr("Restoration failed"), watcher->error().message());
            break;

        case UpgradeStage::EVALUATE_SOFTWARE:
            qCritical() << "Software evaluation error type:" << watcher->error().type();
            qCritical() << "Software evaluation error message:" << watcher->error().message();
            // Ignore NoReply error triggered by cancelation
            if (watcher->error().type() != QDBusError::ErrorType::NoReply)
                emit EvaluateSoftwareError(watcher->error().message());
            break;
        case UpgradeStage::MIGRATION:
            qCritical() << "MigrateError:" << watcher->error().message();
            emit MigrateError(2, watcher->error().message());
            break;

        case UpgradeStage::MIGRATIONLIST:
            emit error(tr("Upgrade failed"), watcher->error().message());
            break;
        }
    }
    else
    {
        if (m_upgradeStage == UpgradeStage::MIGRATION)
        {
            // Handle the struct returned by App Migration.
            MigrateResult result;
            QDBusArgument arg = watcher->reply().arguments().first().value<QDBusArgument>();
            arg >> result;
            if (result.code != 0)
            {
                qCritical() << "Migrate Result code:" << result.code;
                qCritical() << "Migrate Result description:" << result.description;
                emit MigrateError(result.code, result.description);
            }
            else
            {
                emit MigrateDone();
            }
        }
        else if (m_upgradeStage == UpgradeStage::MIGRATIONLIST)
        {
            emit GetMigrateListDone();
        }
    }
}

const QString DBusWorker::GetISOVersion()
{
    return m_isoInter->call("GetISOVersion").arguments().first().toString();
}

const QString DBusWorker::GetDistroVer()
{
    return m_legacySysinfoInter->property("DistroVer").toString();
}

const QString DBusWorker::GetDistroID()
{
    return m_legacySysinfoInter->property("DistroID").toString();
}

const QString DBusWorker::GetVersion()
{
    // Adapt to the latest SystemInfo interface on V23 system
    return m_sysinfoInter->property("Version").toString();
}

void DBusWorker::SendNotification(const QString title, const QString msg)
{
    QStringList in5;
    QVariantMap in6;
    m_notifyInter->call("Notify", QString("系统升级工具"), 0u, QString("upgrade-tool"), title, msg, in5, in6, -1);
}

void DBusWorker::SetPlymouthTheme(QString theme)
{
    m_versionInter->setTimeout(kQDBusAsyncCallTimeout);
    QDBusPendingCall pcall = m_versionInter->asyncCall("SetPlymouthTheme", theme);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    // Bind to error message
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SIGNAL(SetPlymouthThemeDone()));
}

void DBusWorker::RestorePlymouthTheme()
{
    qDebug() << "Restoring Plymouth Theme";
    m_versionInter->setTimeout(kQDBusAsyncCallTimeout);
    m_versionInter->asyncCall("RestorePlymouthTheme");
}

bool DBusWorker::GetIsBackupApps()
{
    return m_configInter->call("GetIsBackupApps").arguments().at(0).toBool();
}

void DBusWorker::CancelBackupApp(bool cancel)
{
    m_appInter->call("CancelBackupApp", cancel);
}

void DBusWorker::MigratePackages()
{
    m_upgradeStage = UpgradeStage::MIGRATION;
    m_appInter->setTimeout(kQDBusAsyncCallTimeout);

    QDBusPendingCall pcall = m_appInter->asyncCall("MigratePackages");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::SetMigrateAppsList(const QStringList &apps)
{
    m_upgradeStage = UpgradeStage::MIGRATIONLIST;
    QDBusPendingCall pcall = m_appInter->asyncCall("SetMigrateAppsList", apps);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onAsyncCallFinished(QDBusPendingCallWatcher*)));
}

void DBusWorker::StopUpgrade()
{
    m_systemupgrade1Inter->call("StopUpgrade");
}

void DBusWorker::CheckISO(const QString &isoPath)
{
    m_isoInter->setTimeout(kQDBusAsyncCallTimeout);

    QDBusPendingCall pcall = m_isoInter->asyncCall("CheckISO", isoPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher *watcher) {
        if (watcher->isError()) {
            emit CheckResult(false);
        }
    });
}
