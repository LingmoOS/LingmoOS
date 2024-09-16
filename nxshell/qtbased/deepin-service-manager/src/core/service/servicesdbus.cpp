// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicesdbus.h"

#include <systemd/sd-bus.h>

#include <QDebug>
#include <QFileInfo>
#include <QLibrary>
#include <QLoggingCategory>
#include <QThread>

Q_LOGGING_CATEGORY(dsm_service_sd, "[SDBusService]")
Q_LOGGING_CATEGORY(dsm_hook_sd, "[SDBusHook]")

QString getCMD(ServiceBase *obj, sd_bus_message *m)
{
    __attribute__((cleanup(sd_bus_creds_unrefp))) sd_bus_creds *creds = NULL;
    ServiceSDBus *srv = qobject_cast<ServiceSDBus *>(obj);
    if (!srv) {
        return "";
    }
    int r;
    pid_t pid;
    r = sd_bus_query_sender_creds(m, SD_BUS_CREDS_PID, &creds);
    if (r < 0)
        return "";

    r = sd_bus_creds_get_pid(creds, &pid);
    if (r < 0)
        return "";
    qCDebug(dsm_hook_sd) << "--pid:" << pid;
    QFile procCmd("/proc/" + QString::number(pid) + "/cmdline");
    QString cmd;
    if (procCmd.open(QIODevice::ReadOnly)) {
        QList<QByteArray> cmds = procCmd.readAll().split('\0');
        cmd = QString(cmds.first());
        qCDebug(dsm_hook_sd) << "--cmd:" << cmd;
    }
    return cmd;
}

int sd_bus_message_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    (void)ret_error;
    const QString &path = sd_bus_message_get_path(m);
    qCInfo(dsm_hook_sd) << QString("--msg= (sender=%2, path=%3, interface=%4, member=%5, "
                                   "signature=%6)")
                               .arg(sd_bus_message_get_sender(m))
                               .arg(sd_bus_message_get_path(m))
                               .arg(sd_bus_message_get_interface(m))
                               .arg(sd_bus_message_get_member(m))
                               .arg(sd_bus_message_get_signature(m, true));

    // int sd_bus_get_tid(sd_bus *bus, pid_t *tid);

    ServiceBase *qobj = static_cast<ServiceBase *>(userdata);
    if (!qobj) {
        return -1;
    }

    if (!qobj->isRegister()) {
        qobj->registerService();
    }
    if (!qobj->policy->isResident() && !qobj->isLockTimer()) {
        qCInfo(dsm_hook_sd) << QString("--service: %1 will unregister in %2 minutes!")
                                   .arg(qobj->policy->name)
                                   .arg(qobj->policy->idleTime);
        qobj->restartTimer();
    }
    const QString &mem = sd_bus_message_get_member(m);
    const QString &interface = sd_bus_message_get_interface(m);
    if (mem == "Introspect" && interface == "org.freedesktop.DBus.Introspectable") {
        if (qobj->policy->checkPathHide(path)) {
            qCDebug(dsm_hook_sd) << "--call Introspect" << path << " ,is hided!";
            return sd_bus_reply_method_return(m, "s", "");
        }
    } else if (mem == "Set" && interface == "org.freedesktop.DBus.Properties") {
        char *interface = nullptr;
        char *propertyName = nullptr;
        sd_bus_message_read(m, "ss", &interface, &propertyName);
        const QString &cmd = getCMD(qobj, m);
        if (!qobj->policy->checkPropertyPermission(cmd, path, interface, propertyName)) {
            qCWarning(dsm_hook_sd)
                << "cmd:" << cmd << "not allowded to set property:" << propertyName;
            return sd_bus_reply_method_errorf(m, SD_BUS_ERROR_ACCESS_DENIED, "Access denied");
        }
    } else if (interface != "org.freedesktop.DBus.Properties"
               && interface != "org.freedesktop.DBus.Introspectable"
               && interface != "org.freedesktop.DBus.Peer") {
        const QString &cmd = getCMD(qobj, m);
        if (!qobj->policy->checkMethodPermission(cmd, path, interface, mem)) {
            qCWarning(dsm_hook_sd) << "cmd:" << cmd << "not allowded to call method:" << mem;
            return sd_bus_reply_method_errorf(m, SD_BUS_ERROR_ACCESS_DENIED, "Access denied");
        }
    }

    return 0;
}

ServiceSDBus::ServiceSDBus(QObject *parent)
    : ServiceBase(parent)
    , m_bus(nullptr)
{
    m_SDKType = SDKType::SD;
}

ServiceSDBus::~ServiceSDBus()
{
    if (this->thread() != nullptr) {
        this->thread()->quit();
        this->thread()->deleteLater();
    }
}

void ServiceSDBus::initThread()
{
    sd_bus_slot *slot = NULL;
    auto ret = m_sessionType == QDBusConnection::SessionBus ? sd_bus_open_user(&m_bus)
                                                            : sd_bus_open_system(&m_bus);
    if (ret < 0) {
        qCWarning(dsm_service_sd) << "open dbus error: " << strerror(-ret);
        return;
    }

    const char *unique;
    sd_bus_get_unique_name(m_bus, &unique);
    qCInfo(dsm_service_sd) << "bus unique:" << QString(unique);

    ret = sd_bus_request_name(m_bus, policy->name.toStdString().c_str(), 0);
    if (ret < 0) {
        qCWarning(dsm_service_sd) << "request name error: " << strerror(-ret);
        return;
    }
    ret = sd_bus_add_filter(m_bus, &slot, sd_bus_message_handler, (void *)this);
    if (ret < 0) {
        qCWarning(dsm_service_sd) << "add filter error: " << strerror(-ret);
        return;
    }

    QFileInfo fileInfo(QString(SERVICE_LIB_DIR) + policy->pluginPath);
    if (QLibrary::isLibrary(fileInfo.absoluteFilePath())) {
        m_library = new QLibrary(fileInfo.absoluteFilePath());
        qCInfo(dsm_service_sd) << "init library:" << fileInfo.absoluteFilePath();
    }

    if (!registerService()) {
        qCWarning(dsm_service_sd) << "register service failed: " << policy->name;
    }

    sd_event *event = NULL;
    ret = sd_event_new(&event);
    ret = sd_bus_attach_event(m_bus, event, SD_EVENT_PRIORITY_NORMAL);
    if (ret < 0) {
        qCWarning(dsm_service_sd) << "failed to attach event: " << strerror(-ret);
    }
    ret = sd_event_loop(event);
    if (ret < 0) {
        qCWarning(dsm_service_sd) << "failed to loop event: " << strerror(-ret);
    }
    ServiceBase::initThread();
}

bool ServiceSDBus::registerService()
{
    qCInfo(dsm_service_sd) << "service register: " << policy->name;

    if (libFuncCall("DSMRegister", true)) {
        ServiceBase::registerService();
        return true;
    } else {
        return false;
    }
}

bool ServiceSDBus::unregisterService()
{
    qCInfo(dsm_service_sd) << "service unregister: " << policy->name;

    if (libFuncCall("DSMUnRegister", false)) {
        ServiceBase::registerService();
        return true;
    } else {
        return false;
    }
}

bool ServiceSDBus::libFuncCall(const QString &funcName, bool isRegister)
{
    if (m_library == nullptr) {
        return false;
    }
    auto objFunc = isRegister ? DSMRegister(m_library->resolve(funcName.toStdString().c_str()))
                              : DSMUnRegister(m_library->resolve(funcName.toStdString().c_str()));
    if (!objFunc) {
        qCWarning(dsm_service_sd)
            << QString("failed to resolve the method: %1\n file: %2\n error message: %3")
                   .arg(funcName)
                   .arg(m_library->fileName())
                   .arg(m_library->errorString());
        if (m_library->isLoaded())
            m_library->unload();
        m_library->deleteLater();
        return false;
    }
    int ret = objFunc(policy->name.toStdString().c_str(), (void *)m_bus);
    if (ret) {
        return false;
    }
    return true;
}
