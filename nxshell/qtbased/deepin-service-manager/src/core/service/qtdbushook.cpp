// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qtdbushook.h"

#include "policy/policy.h"
#include "serviceqtdbus.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QTimer>

#ifdef Q_DBUS_EXPORT
extern Q_DBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage &));
extern Q_DBUS_EXPORT void qDBusAddFilterHook(int (*)(const QString &, const QDBusMessage &));
#else
extern QDBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage &));
extern QDBUS_EXPORT void qDBusAddFilterHook(int (*)(const QString &, const QDBusMessage &));
#endif

#ifdef Q_DBUS_HOOK_FILTER
#  define HOOK_RESULT_TYPE int
#  define HOOK_RESULT_SUCCESS 0
#  define HOOK_RESULT_FAILED -1
#else
#  define HOOK_RESULT_TYPE void
#  define HOOK_RESULT_SUCCESS
#  define HOOK_RESULT_FAILED
#endif

Q_LOGGING_CATEGORY(dsm_hook_qt, "[QDBusHook]")

QString getCMD(ServiceBase *obj, QString dbusService)
{
    ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(obj);
    if (!srv) {
        return "";
    }
    const unsigned int &pid = srv->qDbusConnection().interface()->servicePid(dbusService).value();
    qCDebug(dsm_hook_qt) << "--pid:" << pid;
    QFile procCmd("/proc/" + QString::number(pid) + "/cmdline");
    QString cmd;
    if (procCmd.open(QIODevice::ReadOnly)) {
        QList<QByteArray> cmds = procCmd.readAll().split('\0');
        cmd = QString(cmds.first());
        qCDebug(dsm_hook_qt) << "--cmd:" << cmd;
    }
    return cmd;
}

// if it is not a local message, hook exec at main thread
void QTDBusSpyHook(const QDBusMessage &msg)
{
    qCInfo(dsm_hook_qt) << "--msg=" << msg;

    ServiceBase *serviceObj = nullptr;
    bool isSubPath;
    QString realPath; // 子PATH可能没有配置，使用父PATH的配置
    bool findRet =
        QTDbusHook::instance()->getServiceObject("", msg.path(), &serviceObj, isSubPath, realPath);
    if (!findRet) {
        qCWarning(dsm_hook_qt) << "--can not find hook object: " << msg.path();
        return;
    }
    if (!serviceObj->isRegister()) {
        qCInfo(dsm_hook_qt) << "--to register dbus object: " << msg.path();
        serviceObj->registerService();
    }

    if (!serviceObj->policy->isResident() && !serviceObj->isLockTimer()) {
        qCInfo(dsm_hook_qt) << QString("--service: %1 will unregister in %2 minutes!")
                                   .arg(serviceObj->policy->name)
                                   .arg(serviceObj->policy->idleTime);
        QTimer::singleShot(0, serviceObj, SLOT(restartTimer()));
    }
    if (msg.member() == "Introspect" && msg.interface() == "org.freedesktop.DBus.Introspectable") {
        if (serviceObj->policy->checkPathHide(realPath)) {
            qCInfo(dsm_hook_qt) << "--call Introspect " << msg.path() << " is hided!";
            QList<QVariant> arguments;
            arguments << "";
            QDBusMessage reply = msg.createReply(arguments);
            ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
            if (srv) {
                srv->qDbusConnection().send(reply);
            }
        }
    } else if (msg.member() == "Set" && msg.interface() == "org.freedesktop.DBus.Properties") {
        const QList<QVariant> &args = msg.arguments();
        if (args.size() >= 2) {
            const QString &cmd = getCMD(serviceObj, msg.service());
            if (!serviceObj->policy->checkPropertyPermission(cmd,
                                                             realPath,
                                                             args.at(0).toString(),
                                                             args.at(1).toString())) {
                qCWarning(dsm_hook_qt)
                    << "cmd:" << cmd << "not allowded to set property:" << args.at(1).toString();
                QDBusMessage reply = msg.createErrorReply("org.freedesktop.DBus.Error.AccessDenied",
                                                          "Access denied");
                ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
                if (srv) {
                    srv->qDbusConnection().send(reply);
                    return;
                }
            }
        }
    } else if (msg.interface() != "org.freedesktop.DBus.Properties"
               && msg.interface() != "org.freedesktop.DBus.Introspectable"
               && msg.interface() != "org.freedesktop.DBus.Peer") {
        const QString &cmd = getCMD(serviceObj, msg.service());
        if (!serviceObj->policy->checkMethodPermission(cmd,
                                                       realPath,
                                                       msg.interface(),
                                                       msg.member())) {
            QDBusMessage reply =
                msg.createErrorReply("org.freedesktop.DBus.Error.AccessDenied", "Access denied");
            qCWarning(dsm_hook_qt)
                << "cmd:" << cmd << "not allowded to call method:" << msg.member();
            ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
            if (srv) {
                srv->qDbusConnection().send(reply);
                return;
            }
        }
    }
    return;
}

// if it is not a local message, hook exec at main thread
int QTDBusHook(const QString &baseService, const QDBusMessage &msg)
{
    qCInfo(dsm_hook_qt) << "--baseService=" << baseService;
    qCInfo(dsm_hook_qt) << "--msg=" << msg;

    ServiceBase *serviceObj = nullptr;
    bool isSubPath;
    QString realPath; // 子PATH可能没有配置，使用父PATH的配置
    bool findRet =
        QTDbusHook::instance()->getServiceObject("", msg.path(), &serviceObj, isSubPath, realPath);
    if (!findRet) {
        qCWarning(dsm_hook_qt) << "--can not find hook object:" << msg.path();
        return 0;
    }
    if (!serviceObj->isRegister()) {
        qCInfo(dsm_hook_qt) << "--to register dbus object: " << msg.path();
        serviceObj->registerService();
    }

    if (!serviceObj->policy->isResident() && !serviceObj->isLockTimer()) {
        qCInfo(dsm_hook_qt) << QString("--service: %1 will unregister in %2 minutes!")
                                   .arg(serviceObj->policy->name)
                                   .arg(serviceObj->policy->idleTime);
        QTimer::singleShot(0, serviceObj, SLOT(restartTimer()));
    }
    if (msg.member() == "Introspect" && msg.interface() == "org.freedesktop.DBus.Introspectable") {
        if (serviceObj->policy->checkPathHide(realPath)) {
            qCInfo(dsm_hook_qt) << "--call Introspect " << msg.path() << " is hided!";
            QList<QVariant> arguments;
            arguments << "";
            QDBusMessage reply = msg.createReply(arguments);
            ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
            if (srv) {
                srv->qDbusConnection().send(reply);
            }
            //            ((ServiceQtDBus*)serviceObj)->qDbusConnection().send(reply);
        }
    } else if (msg.member() == "Set" && msg.interface() == "org.freedesktop.DBus.Properties") {
        const QList<QVariant> &args = msg.arguments();
        if (args.size() >= 2) {
            const QString &cmd = getCMD(serviceObj, msg.service());
            if (!serviceObj->policy->checkPropertyPermission(cmd,
                                                             realPath,
                                                             args.at(0).toString(),
                                                             args.at(1).toString())) {
                qCWarning(dsm_hook_qt)
                    << "cmd:" << cmd << "not allowded to set property:" << args.at(1).toString();
                QDBusMessage reply = msg.createErrorReply("org.freedesktop.DBus.Error.AccessDenied",
                                                          "Access denied");
                ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
                if (srv) {
                    srv->qDbusConnection().send(reply);
                    return -1;
                }
            }
        }
    } else if (msg.interface() != "org.freedesktop.DBus.Properties"
               && msg.interface() != "org.freedesktop.DBus.Introspectable"
               && msg.interface() != "org.freedesktop.DBus.Peer") {
        const QString &cmd = getCMD(serviceObj, msg.service());
        if (!serviceObj->policy->checkMethodPermission(cmd,
                                                       realPath,
                                                       msg.interface(),
                                                       msg.member())) {
            qCWarning(dsm_hook_qt)
                << "cmd:" << cmd << "not allowded to call method:" << msg.member();
            QDBusMessage reply =
                msg.createErrorReply("org.freedesktop.DBus.Error.AccessDenied", "Access denied");
            ServiceQtDBus *srv = qobject_cast<ServiceQtDBus *>(serviceObj);
            if (srv) {
                srv->qDbusConnection().send(reply);
                return -1;
            }
        }
    }
    return 0;

    // test TODO
    //    if (msg.member() == "Register") {
    //        Policy ppp;
    //        ppp.Test();
    //        QList<QVariant> arguments;
    //        arguments << true << "sdvvv";

    //        QDBusMessage reply =
    //        msg.createErrorReply("com.deepin.services.Nooooooo", "The method
    //        call 'Register()' is not supported");
    //        QDBusConnection::connectToBus(QDBusConnection::SessionBus,
    //        "org.deepin.services.demo2").send(reply);
    //    }
}

Q_GLOBAL_STATIC(QTDbusHook, qtDBusHook)

QTDbusHook::QTDbusHook()
{
    qCDebug(dsm_hook_qt) << "qt hook register.";
#ifdef Q_DBUS_HOOK_FILTER
    qDBusAddFilterHook(QTDBusHook);
#else
    qDBusAddSpyHook(QTDBusSpyHook);
#endif
}

QTDbusHook *QTDbusHook::instance()
{
    return qtDBusHook;
}

bool QTDbusHook::getServiceObject(
    QString name, QString path, ServiceBase **service, bool &isSubPath, QString &realPath)
{
    Q_UNUSED(name) // TODO:QtDBus Hook 无法获取到name
    ServiceObjectMap::iterator iterService = m_serviceMap.find(path);
    if (iterService != m_serviceMap.end()) {
        *service = iterService.value();
        isSubPath = true;
        realPath = iterService.key();
        return true;
    }
    for (auto iter = m_serviceMap.begin(); iter != m_serviceMap.end(); ++iter) {
        if (path.startsWith(iter.key()) && iter.value()->policy->allowSubPath(iter.key())) {
            *service = iter.value();
            isSubPath = false;
            realPath = iter.key();
            return true;
        }
    }
    return false;
}

bool QTDbusHook::setServiceObject(ServiceBase *obj)
{
    QStringList paths = obj->policy->paths();
    for (auto path : paths) {
        ServiceObjectMap::iterator iterService = m_serviceMap.find(path);
        if (iterService != m_serviceMap.end()) {
            qCWarning(dsm_hook_qt) << "set service path failed, the object is existed: " << path;
            continue;
        }
        m_serviceMap[path] = obj;
    }
    return true;
}
