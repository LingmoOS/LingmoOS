#include "servicemanager.h"

#include "servicecommon.h"
#include "kerror.h"
#include "serviceinfo.h"
#include "servicefileinfo.h"
#include "systemd1serviceinterface.h"
#include "systemd1managerinterface.h"
#include "servicedataworker.h"

#include <QDebug>
#include <QList>
#include <QString>
#include <QtDBus>
#include <QTimer>
#include <QThread>
#include <QApplication>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define EXEC_SYSV_CHECK "/lib/systemd/systemd-sysv-install"
#define BIN_PKEXEC_PATH "/usr/bin/pkexec"
#define BIN_SYSTEMCTL_PATH "/usr/bin/systemctl"

std::atomic<ServiceManager *> ServiceManager::m_instance;
std::mutex ServiceManager::m_mutex;

DelayUpdateTimer::DelayUpdateTimer(ServiceManager *mgr, QObject *parent)
    : QObject(parent), m_mgr(mgr)
{
    m_timer = new QTimer(parent);
}

void DelayUpdateTimer::start(const QString &path)
{
    connect(m_timer, &QTimer::timeout, this, [ = ]() {
        ServiceInfo info = m_mgr->updateServiceInfo(path);
        if (m_cnt >= 8 || ServiceManager::isFinalState(info.getActiveState())) {
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
    ServiceInfo::registerMetaType();
    ServiceFileInfo::registerMetaType();

    m_worker = new ServiceDataWorker();
    m_worker->moveToThread(&m_workerThread);
    connect(this, &ServiceManager::updateList, m_worker, &ServiceDataWorker::refreshServiceList);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &ServiceDataWorker::serviceListReady, this, &ServiceManager::serviceListUpdated);
    m_workerThread.start();
}

ServiceManager::~ServiceManager()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

ServiceManager* ServiceManager::instance()
{
    ServiceManager *sin = m_instance.load();
    if (!sin) {
        std::lock_guard<std::mutex> lock(m_mutex);
        sin = m_instance.load();
        if (!sin) {
            sin = new ServiceManager();
            m_instance.store(sin);
        }
    }
    return sin;
}

// 服务是否为初始化使能状态
bool ServiceManager::isSysVInitEnabled(const QString &sname)
{
    int rc = QProcess::execute(EXEC_SYSV_CHECK, {"is-enabled", sname});
    // from /lib/systemd/systemd-sysv-install#is-enabled
    if (rc == 0) {
        return true; // enabled
    } else {
        return false;
    }
}

QString ServiceManager::getServiceStartupMode(const QString &id, const QString &state)
{
    QString startupType = "";
    if (isServiceNoOp(state) || id.endsWith("@")) {
        startupType = "N/A";
    } else {
        bool b = isServiceAutoStartup(id, state);
        if (b) {
            startupType = "Auto";
        } else {
            startupType = "Manual";
        }
    }
    return startupType;
}

bool ServiceManager::isFinalState(QString strActiveState)
{
    if (strActiveState != "active" ||
        strActiveState != "inactive" ||
        strActiveState != "failed") {
        return false;
    }
    return true;
}

bool ServiceManager::isActiveState(QString strActiveState)
{
    if (strActiveState != "active") {
        return false;
    }
    return true;
}

bool ServiceManager::isServiceNoOp(const QString &state)
{
    return (state.isEmpty() ||
            state == "enabled-runtime" ||
            state == "static" ||
            state == "transient" ||
            state == "indirect" ||
            state == "masked");
}

bool ServiceManager::isServiceAutoStartup(const QString &id, const QString &state)
{
    if (state == "generated" || state == "indirect") {
        return isSysVInitEnabled(id);
    } else {
        return (!state.isEmpty() && (state == "enabled" ||
                                     state == "enabled-runtime" ||
                                     state == "static"));
    }
}

void ServiceManager::updateServiceList()
{
    Q_EMIT updateList();
}

QString ServiceManager::normalizeServiceId(const QString &id, const QString &param)
{
    QString buf = id;
    if (buf.endsWith(UNIT_TYPE_SERVICE_SUFFIX)) {
        if (buf.lastIndexOf('@') > 0) {
            if (!param.isEmpty())
                buf = QString("%1%2%3")
                      .arg(buf.left(buf.lastIndexOf('@') + 1))
                      .arg(param)
                      .arg(UNIT_TYPE_SERVICE_SUFFIX);
        }
    } else {
        if (buf.endsWith('@') && !param.isEmpty()) {
            buf = QString("%1%2%3").arg(buf).arg(param).arg(UNIT_TYPE_SERVICE_SUFFIX);
        } else {
            buf = buf.append(UNIT_TYPE_SERVICE_SUFFIX);
        }
    }

    return buf;
}

KError ServiceManager::startService(const QString &id,
                                          const QString &param)
{
    KError ke;
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE_NAME,
                                   DBUS_SYSTEMD1_SERVICE_PATH,
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id, param);
    QString mode = "replace";

    auto oResult = iface.StartService(buf, mode);
    ke = oResult.first;
    if (ke) {
        qDebug() << "StartService failed:" << buf << ke.getErrorName() << ke.getErrorMessage();
        return ke;
    }
    QDBusObjectPath objectPath = oResult.second;

    if (id.endsWith("@"))
        return {};

    oResult = iface.GetService(buf);
    ke = oResult.first;
    ServiceInfo service;
    if (ke) {
        if (ke.getCode() == 3) {
            auto objectPath1 = Systemd1ServiceInterface::normalizeUnitPath(buf);
            service = updateServiceInfo(objectPath1.path());
        } else {
            qDebug() << "GetUnit failed:" << buf << ke.getErrorName() << ke.getErrorMessage();
            return ke;
        }
    } else {
        objectPath = oResult.second;
        service = updateServiceInfo(objectPath.path());
    }

    if (!ServiceManager::isFinalState(service.getActiveState())) {
        auto *timer = new DelayUpdateTimer(this);
        timer->start(objectPath.path());
    }
    return {};
}

KError ServiceManager::stopService(const QString &id)
{
    KError ke;
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE_NAME,
                                   DBUS_SYSTEMD1_SERVICE_PATH,
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id);
    QString mode = "replace";

    auto oResult = iface.StopService(buf, mode);
    ke = oResult.first;
    if (ke) {
        qDebug() << "Stop Service failed:" << ke.getErrorName() << ke.getErrorMessage();
        return ke;
    }
    QDBusObjectPath objectPath = oResult.second;

    if (id.endsWith("@"))
        return {};

    oResult = iface.GetService(buf);
    ke = oResult.first;
    ServiceInfo service;
    if (ke) {
        if (ke.getCode() == 3) {
            auto objectPath1 = Systemd1ServiceInterface::normalizeUnitPath(buf);
            service = updateServiceInfo(objectPath1.path());
        } else {
            qDebug() << "Get Service failed:" << ke.getErrorName() << ke.getErrorMessage();
            return ke;
        }
    } else {
        objectPath = oResult.second;
        service = updateServiceInfo(objectPath.path());
    }

    if (!ServiceManager::isFinalState(service.getActiveState())) {
        auto *timer = new DelayUpdateTimer(this);
        timer->start(objectPath.path());
    }
    return {};
}

KError ServiceManager::restartService(const QString &id, const QString &param)
{
    KError ke;
    Systemd1ManagerInterface iface(DBUS_SYSTEMD1_SERVICE_NAME,
                                   DBUS_SYSTEMD1_SERVICE_PATH,
                                   QDBusConnection::systemBus());

    auto buf = normalizeServiceId(id, param);
    QString mode = "replace";

    auto oResult = iface.RestartService(buf, mode);
    ke = oResult.first;
    if (ke) {
        qDebug() << "Restart Service failed:" << buf << ke.getErrorName() << ke.getErrorMessage();
        return ke;
    }
    QDBusObjectPath objectPath = oResult.second;

    if (id.endsWith("@"))
        return {};

    oResult = iface.GetService(buf);
    ke = oResult.first;
    ServiceInfo service;
    if (ke) {
        if (ke.getCode() == 3) {
            auto objectPath1 = Systemd1ServiceInterface::normalizeUnitPath(buf);
            service = updateServiceInfo(objectPath1.path());
        } else {
            qDebug() << "Get Unit failed:" << buf << ke.getErrorName() << ke.getErrorMessage();
            return ke;
        }
    } else {
        objectPath = oResult.second;
        service = updateServiceInfo(objectPath.path());
    }

    if (!ServiceManager::isFinalState(service.getActiveState())) {
        auto *timer = new DelayUpdateTimer(this);
        timer->start(objectPath.path());
    }

    return {};
}

KError ServiceManager::setServiceStartupMode(const QString &id, bool autoStart)
{
    KError ke;

    auto errfmt = [ = ](KError & pe, decltype(errno) err, const QString & title, const QString &message = {}) -> KError & {
        pe.setCode(KError::kErrorSystem);
        pe.setSubCode(err);
        pe.setErrorName(title);
        auto errmsg = (err != 0 ? QString("Error: [%1] %2").arg(err).arg(strerror(err)) : QString("Error: "));
        if (!message.isEmpty())
        {
            errmsg = QString("%1 - %2").arg(errmsg).arg(message);
        }
        pe.setErrorMessage(errmsg);
        return pe;
    };
    const QString title = tr("Failed to set service startup type");
    int rc = 0;
    struct stat sbuf;

    // check pkexec existence
    errno = 0;
    sbuf = {};
    rc = stat(BIN_PKEXEC_PATH, &sbuf);
    if (rc == -1) {
        qDebug() << "check pkexec existence failed";
        ke = errfmt(ke, errno, title, BIN_PKEXEC_PATH);
        return ke;
    }

    // check systemctl existence
    errno = 0;
    sbuf = {};
    rc = stat(BIN_SYSTEMCTL_PATH, &sbuf);
    if (rc == -1) {
        qDebug() << "check systemctl existence failed";
        ke = errfmt(ke, errno, title, BIN_SYSTEMCTL_PATH);
        return ke;
    }

    sighandler_t SignalOld;
    SignalOld = signal(SIGCHLD, SIG_DFL);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    // {BIN_PKEXEC_PATH} {BIN_SYSTEMCTL_PATH} {enable/disable} {service}
    QString action = autoStart ? "enable" : "disable";
    proc.start(BIN_PKEXEC_PATH, {BIN_SYSTEMCTL_PATH, action, id});
    proc.waitForFinished(-1);
    auto exitStatus = proc.exitStatus();
    signal(SIGCHLD, SignalOld);
    KError le;
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
        le = errfmt(le, errno, title, tr("Error: Failed to set service startup type due to the crashed sub process."));
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
        } else {
            // success - refresh service stat -send signal

            // special case, do nothing there
            if (id.endsWith("@"))
                return {};

            Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                           DBUS_SYSTEMD1_SERVICE_PATH,
                                           QDBusConnection::systemBus());
            auto buf = normalizeServiceId(id, {});
            auto re = mgrIf.GetService(buf);
            le = re.first;
            if (le) {
                if (le.getCode() == 3) {
                    auto o = Systemd1ServiceInterface::normalizeUnitPath(buf);
                    updateServiceInfo(o.path());
                } else {
                    return le;
                }
            } else {
                updateServiceInfo(re.second.path());
            }
        }
    }
    return {};
}

ServiceInfo ServiceManager::updateServiceInfo(const QString &opath)
{
    KError ke;
    ServiceInfo service;

    Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                   DBUS_SYSTEMD1_SERVICE_PATH,
                                   QDBusConnection::systemBus());
    Systemd1ServiceInterface svcIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                   opath,
                                   QDBusConnection::systemBus());

    auto idResult = svcIf.getId();
    ke = idResult.first;
    QString id = idResult.second;
    auto sname = id;
    if (ke) {
        qDebug() << "getId failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        if (sname.endsWith(UNIT_TYPE_SERVICE_SUFFIX)) {
            sname.chop(strlen(UNIT_TYPE_SERVICE_SUFFIX));
        }
        service.setID(id);
        service.setSName(sname);
    }

    auto loadStateResult = svcIf.getLoadState();
    ke = loadStateResult.first;
    if (ke) {
        qDebug() << "getLoadState failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setLoadState(loadStateResult.second);
    }

    auto activeStateResult = svcIf.getActiveState();
    ke = activeStateResult.first;
    if (ke) {
        qDebug() << "getActiveState failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setActiveState(activeStateResult.second);
    }

    auto subStateResult = svcIf.getSubState();
    ke = subStateResult.first;
    if (ke) {
        qDebug() << "getSubState failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setSubState(subStateResult.second);
    }

    auto unitFileStateResult = mgrIf.GetServiceFileState(id);
    ke = unitFileStateResult.first;
    if (ke) {
        qDebug() << "getUnitFileState failed:" << service.getID() << ke.getErrorName()
                 << ke.getErrorMessage();
    } else {
        service.setState(unitFileStateResult.second);
        service.setStartupType(ServiceManager::getServiceStartupMode(
                                 service.getSName(),
                                 service.getState()));
    }

    auto descResult = svcIf.getDescription();
    ke = descResult.first;
    if (ke) {
        qDebug() << "getDescription failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setDescription(descResult.second);
    }

    auto mainPIDResult = svcIf.getMainPID();
    ke = mainPIDResult.first;
    if (ke) {
        qDebug() << "getMainPID failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setMainPID(mainPIDResult.second);
    }

    auto canStartResult = svcIf.canStart();
    ke = canStartResult.first;
    if (ke) {
        qDebug() << "canStart failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setIsCanStart(canStartResult.second);
    }

    auto canStopResult = svcIf.canStop();
    ke = canStopResult.first;
    if (ke) {
        qDebug() << "canStop failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setIsCanStop(canStopResult.second);
    }

    auto canReloadResult = svcIf.canReload();
    ke = canReloadResult.first;
    if (ke) {
        qDebug() << "canReload failed:" << ke.getErrorName() << ke.getErrorMessage();
    } else {
        service.setIsCanReload(canReloadResult.second);
    }

    Q_EMIT serviceSingleUpdated(service);

    return service;
}
