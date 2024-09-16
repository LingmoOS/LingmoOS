// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "service_manager.h"
#include "ddlog.h"
#include "application.h"
#include "dbus/dbus_common.h"
#include "dbus/dbus_properties_interface.h"
#include "dbus/environment_file.h"
#include "dbus/systemd1_manager_interface.h"
#include "dbus/systemd1_service_interface.h"
#include "dbus/systemd1_unit_interface.h"
#include "dbus/unit_file_info.h"
#include "dbus/unit_info.h"
#include "service/system_service_entry.h"
#include "service/service_manager_worker.h"

#include <DApplication>
#include <DLog>

#include <QAbstractItemModel>
#include <QHash>
#include <QList>
#include <QString>
#include <QtDBus>
#include <QTimer>
#include <QDBusConnection>
#include <QDBusInterface>

#include <memory>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define BIN_PKEXEC_PATH "/usr/bin/pkexec"
#define BIN_SYSTEMCTL_PATH "/usr/bin/systemctl"

DCORE_USE_NAMESPACE
using namespace dbus::common;
using namespace DDLog;
std::atomic<ServiceManager *> ServiceManager::m_instance;
std::mutex ServiceManager::m_mutex;

/**
   @brief 非开发者模式下，使用后端 DBus 服务设置 systemd 服务 \a serviceName 的启动模式
 */
static bool setServiceEnable(const QString &servieName, bool enable, QString &errorString)
{
    QDBusInterface interface("org.deepin.SystemMonitorSystemServer",
                             "/org/deepin/SystemMonitorSystemServer",
                             "org.deepin.SystemMonitorSystemServer",
                             QDBusConnection::systemBus());
    QDBusReply<QString> retMsg = interface.call("setServiceEnable", servieName, enable);
    errorString.clear();
    if (!retMsg.isValid()) {
        errorString = retMsg.error().message();
    } else if (!retMsg.value().isEmpty()) {
        errorString = retMsg.value();
    }

    if (!errorString.isEmpty()) {
        qCWarning(app) << QString("Set service %1 failed, error %1").arg(enable ? "enable" : "disable").arg(errorString);
        return false;
    } else {
        qCDebug(app) << QString("Set service %1 ret: %2").arg(enable ? "enable" : "disable").arg(retMsg.value());
        return true;
    }
}

CustomTimer::CustomTimer(ServiceManager *mgr, QObject *parent)
    : QObject(parent), m_mgr(mgr)
{
    m_timer = new QTimer(parent);
}

void CustomTimer::start(const QString &path)
{
    connect(m_timer, &QTimer::timeout, this, [=]() {
        SystemServiceEntry e = m_mgr->updateServiceEntry(path);
        if (m_cnt >= 6 || isFinalState(e.getActiveState().toLocal8Bit())) {
            m_timer->stop();
            this->deleteLater();
        } else {
            ++m_cnt;
            m_timer->stop();
            m_timer->start(m_cnt * m_cnt * 200);
        }
    });
    m_timer->start(200);
}

ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
{
    UnitInfo::registerMetaType();
    UnitFileInfo::registerMetaType();
    EnvironmentFile::registerMetaType();

    qRegisterMetaType<QList<SystemServiceEntry>>("ServiceEntryList");

    m_worker = new ServiceManagerWorker();
    m_worker->moveToThread(&m_workerThread);
    connect(this, &ServiceManager::beginUpdateList, m_worker, &ServiceManagerWorker::startJob);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &ServiceManagerWorker::resultReady, this, &ServiceManager::serviceListUpdated);
    m_workerThread.start();
}

ServiceManager::~ServiceManager()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void ServiceManager::updateServiceList()
{
    Q_EMIT beginUpdateList();
}

QString ServiceManager::normalizeServiceId(const QString &id, const QString &param)
{
    QString buf = id;
    if (buf.endsWith(UnitTypeServiceSuffix)) {
        if (buf.lastIndexOf('@') > 0) {
            if (!param.isEmpty())
                buf = QString("%1%2%3")
                              .arg(buf.left(buf.lastIndexOf('@') + 1))
                              .arg(param)
                              .arg(UnitTypeServiceSuffix);
        }
    } else {
        if (buf.endsWith('@') && !param.isEmpty()) {
            buf = QString("%1%2%3").arg(buf).arg(param).arg(UnitTypeServiceSuffix);
        } else {
            buf = buf.append(UnitTypeServiceSuffix);
        }
    }

    return buf;
}

ErrorContext ServiceManager::startService(const QString &id,
                                          const QString &param)
{
    ErrorContext ec {};
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE,
                                   kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id, param);
    auto mode = UnitControlJobModeMap.value(kJobModeReplace);

    auto oResult = iface.StartUnit(buf, mode);
    ec = oResult.first;
    if (ec) {
        qCDebug(app) << "StartUnit failed:" << buf << ec.getErrorName() << ec.getErrorMessage();
        return ec;
    }
    QDBusObjectPath o = oResult.second;

    if (id.endsWith("@"))
        return ErrorContext();

    oResult = iface.GetUnit(buf);
    ec = oResult.first;
    SystemServiceEntry entry;
    if (ec) {
        if (ec.getCode() == 3) {
            auto o1 = Systemd1UnitInterface::normalizeUnitPath(buf);
            entry = updateServiceEntry(o1.path());
        } else {
            qCDebug(app) << "GetUnit failed:" << buf << ec.getErrorName() << ec.getErrorMessage();
            return ec;
        }
    } else {
        o = oResult.second;
        entry = updateServiceEntry(o.path());
    }

    if (!isFinalState(entry.getActiveState().toLocal8Bit())) {
        auto *timer = new CustomTimer { this };
        timer->start(o.path());
    }

    return ErrorContext();
}

ErrorContext ServiceManager::stopService(const QString &id)
{
    ErrorContext ec {};
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE,
                                   kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id);
    auto mode = UnitControlJobModeMap.value(kJobModeReplace);

    auto oResult = iface.StopUnit(buf, mode);
    ec = oResult.first;
    if (ec) {
        qCDebug(app) << "Stop Unit failed:" << ec.getErrorName() << ec.getErrorMessage();
        return ec;
    }
    QDBusObjectPath o = oResult.second;

    if (id.endsWith("@"))
        return ErrorContext();

    oResult = iface.GetUnit(buf);
    ec = oResult.first;
    SystemServiceEntry entry;
    if (ec) {
        if (ec.getCode() == 3) {
            auto o1 = Systemd1UnitInterface::normalizeUnitPath(buf);
            entry = updateServiceEntry(o1.path());
        } else {
            qCDebug(app) << "Get Unit failed:" << ec.getErrorName() << ec.getErrorMessage();
            return ec;
        }
    } else {
        o = oResult.second;
        entry = updateServiceEntry(o.path());
    }

    if (!isFinalState(entry.getActiveState().toLocal8Bit())) {
        auto *timer = new CustomTimer { this };
        timer->start(o.path());
    }

    return ErrorContext();
}

ErrorContext ServiceManager::restartService(const QString &id, const QString &param)
{
    ErrorContext ec {};
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE,
                                   kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id, param);
    auto mode = UnitControlJobModeMap.value(kJobModeReplace);

    auto oResult = iface.RestartUnit(buf, mode);
    ec = oResult.first;
    if (ec) {
        qCDebug(app) << "Restart Unit failed:" << buf << ec.getErrorName() << ec.getErrorMessage();
        return ec;
    }
    QDBusObjectPath o = oResult.second;

    if (id.endsWith("@"))
        return ErrorContext();

    oResult = iface.GetUnit(buf);
    ec = oResult.first;
    SystemServiceEntry entry;
    if (ec) {
        if (ec.getCode() == 3) {
            auto o1 = Systemd1UnitInterface::normalizeUnitPath(buf);
            entry = updateServiceEntry(o1.path());
        } else {
            qCDebug(app) << "Get Unit failed:" << buf << ec.getErrorName() << ec.getErrorMessage();
            return ec;
        }
    } else {
        o = oResult.second;
        entry = updateServiceEntry(o.path());
    }

    if (!isFinalState(entry.getActiveState().toLocal8Bit())) {
        auto *timer = new CustomTimer { this };
        timer->start(o.path());
    }

    return ErrorContext();
}

ErrorContext ServiceManager::setServiceStartupMode(const QString &id, bool autoStart)
{
    ErrorContext ec {};

#ifdef USE_POLICYKIT1_AUTHORITy_API
    // check sysv script

    // enable/disable with EnableUnitFiles/DisableUnitFiles
    Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE, kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());
    if (autoStart) {
        auto re = mgrIf.EnableUnitFiles({ entry.getId() });
        ec = re.first;
        if (ec) {
            qCDebug(app) << "call EnableUnitFiles failed:" << ec.getErrorName() << ec.getErrorMessage();
            return ec;
        }
        auto eResult = re.second;
    } else {
        auto re = mgrIf.DisableUnitFiles({ entry.getId() });
        ec = re.first;
        if (ec) {
            qCDebug(app) << "call DisableUnitFiles failed:" << ec.getErrorName() << ec.getErrorMessage();
            return ec;
        }
        auto dResult = re.second;
    }
#else
    auto errfmt = [=](ErrorContext &pe, decltype(errno) err, const QString &title, const QString &message = {}) -> ErrorContext & {
        pe.setCode(ErrorContext::kErrorTypeSystem);
        pe.setSubCode(err);
        pe.setErrorName(title);
        auto errmsg = (err != 0 ? QString("Error: [%1] %2").arg(err).arg(strerror(err)) : QString("Error: "));
        if (!message.isEmpty()) {
            errmsg = QString("%1 - %2").arg(errmsg).arg(message);
        }
        pe.setErrorMessage(errmsg);
        return pe;
    };
    const QString title = QApplication::translate("Service.Action.Set.Startup.Mode",
                                                  "Failed to set service startup type");
    int rc {};
    struct stat sbuf;

    // check pkexec existence
    errno = 0;
    sbuf = {};
    rc = stat(BIN_PKEXEC_PATH, &sbuf);
    if (rc == -1) {
        qCDebug(app) << "check pkexec existence failed";
        ec = errfmt(ec, errno, title, BIN_PKEXEC_PATH);
        return ec;
    }

    // check systemctl existence
    errno = 0;
    sbuf = {};
    rc = stat(BIN_SYSTEMCTL_PATH, &sbuf);
    if (rc == -1) {
        qCDebug(app) << "check systemctl existence failed";
        ec = errfmt(ec, errno, title, BIN_SYSTEMCTL_PATH);
        return ec;
    }

    QFile file("/var/lib/deepin/developer-mode/enabled");
    bool developerMode = false;
    if (file.open(QIODevice::ReadOnly)) {
        QString lineStr = file.readLine();
        developerMode = (!lineStr.isEmpty() && lineStr.trimmed() == "1");
    }
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    // {BIN_PKEXEC_PATH} {BIN_SYSTEMCTL_PATH} {enable/disable} {service}
    QString action = autoStart ? "enable" : "disable";
    bool useProcess = true;
    if (developerMode) {
        proc.start(BIN_PKEXEC_PATH, { BIN_SYSTEMCTL_PATH, action, id });
    } else {
        // Bug 241793 非开发者模式，使用后端DBus服务设置启动方式
#if 0
        proc.start(BIN_SYSTEMCTL_PATH, {action, id});
#else
        useProcess = false;
        QString errorString;
        bool dbusRet = setServiceEnable(id, autoStart, errorString);
        if (!dbusRet) {
            errno = 0;
            ErrorContext errCtx {};
            errCtx = errfmt(errCtx, errno, title, errorString);
            return errCtx;
        }
#endif
    }

    ErrorContext le {};
    if (useProcess) {
        proc.waitForFinished(-1);
        auto exitStatus = proc.exitStatus();

        // exitStatus:
        //      crashed
        //
        // exitCode:
        //      127 (pkexec) - not auth/cant auth/error
        //      126 (pkexec) - auth dialog dismissed
        //      0 (systemctl) - ok
        //      !0 (systemctl) - systemctl error, read stdout from child process
        if (exitStatus == QProcess::CrashExit) {
            errno = 0;
            le = errfmt(le, errno, title, QApplication::translate("Service.Action.Set.Startup.Mode", "Error: Failed to set service startup type due to the crashed sub process."));
            return le;
        } else {
            auto exitCode = proc.exitCode();
            if (exitCode == 127 || exitCode == 126) {
                errno = EPERM;
                le = errfmt(le, errno, title);
                return le;
            } else if (exitCode != 0) {
                auto buf = proc.readAllStandardOutput();
                errno = 0;
                le = errfmt(le, errno, title, buf);
                return le;
            }
        }
    }

    {
        // success - refresh service stat -send signal

        // special case, do nothing there
        if (id.endsWith("@"))
            return ErrorContext();

        Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE,
                                       kSystemDObjectPath.path(),
                                       QDBusConnection::systemBus());
        auto buf = normalizeServiceId(id, {});
        auto re = mgrIf.GetUnit(buf);
        le = re.first;
        if (le) {
            if (le.getCode() == 3) {
                auto o = Systemd1UnitInterface::normalizeUnitPath(buf);
                updateServiceEntry(o.path());
            } else {
                return le;
            }
        } else {
            updateServiceEntry(re.second.path());
        }
    }
#endif
    return ErrorContext();
}

SystemServiceEntry ServiceManager::updateServiceEntry(const QString &opath)
{
    ErrorContext ec;
    SystemServiceEntry entry {};

    Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE,
                                   kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());
    Systemd1UnitInterface unitIf(DBUS_SYSTEMD1_SERVICE,
                                 opath,
                                 QDBusConnection::systemBus());
    Systemd1ServiceInterface svcIf(DBUS_SYSTEMD1_SERVICE,
                                   opath,
                                   QDBusConnection::systemBus());

    auto idResult = unitIf.getId();
    ec = idResult.first;
    QString id = idResult.second;
    auto sname = id;
    if (ec) {
        qCDebug(app) << "getId failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        if (sname.endsWith(UnitTypeServiceSuffix)) {
            sname.chop(strlen(UnitTypeServiceSuffix));
        }
        entry.setId(id);
        entry.setSName(sname);
    }

    auto loadStateResult = unitIf.getLoadState();
    ec = loadStateResult.first;
    if (ec) {
        qCDebug(app) << "getLoadState failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setLoadState(loadStateResult.second);
    }

    auto activeStateResult = unitIf.getActiveState();
    ec = activeStateResult.first;
    if (ec) {
        qCDebug(app) << "getActiveState failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setActiveState(activeStateResult.second);
    }

    auto subStateResult = unitIf.getSubState();
    ec = subStateResult.first;
    if (ec) {
        qCDebug(app) << "getSubState failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setSubState(subStateResult.second);
    }

    auto unitFileStateResult = mgrIf.GetUnitFileState(id);
    ec = unitFileStateResult.first;
    if (ec) {
        qCDebug(app) << "getUnitFileState failed:" << entry.getId() << ec.getErrorName()
                     << ec.getErrorMessage();
    } else {
        entry.setState(unitFileStateResult.second);
        entry.setStartupType(ServiceManager::getServiceStartupType(
                entry.getSName(),
                entry.getState()));
    }

    auto descResult = unitIf.getDescription();
    ec = descResult.first;
    if (ec) {
        qCDebug(app) << "getDescription failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setDescription(descResult.second);
    }

    auto mainPIDResult = svcIf.getMainPID();
    ec = mainPIDResult.first;
    if (ec) {
        qCDebug(app) << "getMainPID failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setMainPID(mainPIDResult.second);
    }

    auto canStartResult = unitIf.canStart();
    ec = canStartResult.first;
    if (ec) {
        qCDebug(app) << "canStart failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setCanStart(canStartResult.second);
    }

    auto canStopResult = unitIf.canStop();
    ec = canStopResult.first;
    if (ec) {
        qCDebug(app) << "canStop failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setCanStop(canStopResult.second);
    }

    auto canReloadResult = unitIf.canReload();
    ec = canReloadResult.first;
    if (ec) {
        qCDebug(app) << "canReload failed:" << ec.getErrorName() << ec.getErrorMessage();
    } else {
        entry.setCanReload(canReloadResult.second);
    }

    Q_EMIT serviceStatusUpdated(entry);

    return entry;
}
