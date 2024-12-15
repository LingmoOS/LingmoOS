// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "serviceqtdbus.h"

#include "policy/policy.h"
#include "qtdbushook.h"

#include <QDBusAbstractAdaptor>
#include <QDebug>
#include <QFileInfo>
#include <QLibrary>
#include <QLoggingCategory>
#include <QMetaClassInfo>
#include <QThread>

Q_LOGGING_CATEGORY(dsm_service_qt, "[QDBusService]")

ServiceQtDBus::ServiceQtDBus(QObject *parent)
    : ServiceBase(parent)
    , m_library(nullptr)
{
    m_SDKType = SDKType::QT;
}

QDBusConnection ServiceQtDBus::qDbusConnection()
{
    if (policy->name.isEmpty()) {
        if (m_sessionType == QDBusConnection::SystemBus) {
            return QDBusConnection::systemBus();
        } else {
            return QDBusConnection::sessionBus();
        }
    } else {
        if (m_sessionType == QDBusConnection::SystemBus) {
            return QDBusConnection::connectToBus(QDBusConnection::SystemBus, policy->name);
        } else {
            return QDBusConnection::connectToBus(QDBusConnection::SessionBus, policy->name);
        }
    }
}

void ServiceQtDBus::initThread()
{
    qCInfo(dsm_service_qt) << "init service: " << policy->name << "paths: " << policy->paths();

    // TODO:无权限、隐藏、按需启动需求的service，不应该注册，避免触发hook，提高效率
    QTDbusHook::instance()->setServiceObject(this);

    QFileInfo fileInfo(QString(SERVICE_LIB_DIR) + policy->pluginPath);
    if (QLibrary::isLibrary(fileInfo.absoluteFilePath())) {
        qCInfo(dsm_service_qt) << "init library: " << fileInfo.absoluteFilePath();
        m_library = new QLibrary(fileInfo.absoluteFilePath());
    }

    if (!registerService()) {
        qCWarning(dsm_service_qt) << "register service failed: " << policy->name;
    }
    qDbusConnection().registerService(policy->name);
    ServiceBase::initThread();
}

bool ServiceQtDBus::registerService()
{
    qCInfo(dsm_service_qt) << "service register: " << policy->name;

    if (libFuncCall("DSMRegister", true)) {
        ServiceBase::registerService();
        return true;
    } else {
        return false;
    }
}

bool ServiceQtDBus::unregisterService()
{
    qCInfo(dsm_service_qt) << "service unregister: " << policy->name;
    if (policy->dbus) {
        delete policy->dbus;
        policy->dbus = nullptr;
    }

    if (libFuncCall("DSMUnRegister", false)) {
        ServiceBase::unregisterService();
        return true;
    } else {
        return false;
    }
}

bool ServiceQtDBus::libFuncCall(const QString &funcName, bool isRegister)
{
    if (m_library == nullptr) {
        return false;
    }
    auto objFunc = isRegister ? DSMRegister(m_library->resolve(funcName.toStdString().c_str()))
                              : DSMUnRegister(m_library->resolve(funcName.toStdString().c_str()));
    if (!objFunc) {
        qCWarning(dsm_service_qt)
                << QString("failed to resolve the method: %1\n file: %2\n error message: %3")
                           .arg(funcName)
                           .arg(m_library->fileName())
                           .arg(m_library->errorString());
        if (m_library->isLoaded())
            m_library->unload();
        m_library->deleteLater();
        return false;
    }
    policy->dbus = new QDBusConnection( qDbusConnection());
    int ret = objFunc(policy->name.toStdString().c_str(), (void *)policy->dbus);
    if (ret) {
        return false;
    }
    return true;
}
