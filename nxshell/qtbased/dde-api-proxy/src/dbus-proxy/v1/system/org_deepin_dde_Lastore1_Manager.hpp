// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "org_deepin_dde_Lastore1_Job.hpp"
#include "system_org_deepin_dde_Lastore1_Manager.h"
#include "system_org_deepin_dde_Lastore1_Updater.h"

class SystemLastore1ManagerProxy : public DBusProxyBase {
public:
    SystemLastore1ManagerProxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr) 
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        InitFilterProperies(QStringList({}));
        InitFilterMethods(QStringList({"GetArchivesInfo", "UpdateSource", "CleanJob", "PackageInstallable", "InstallPackage", "PackageExists"}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::lastore1::Manager(m_dbusName, m_dbusPath, QDBusConnection::systemBus(), this);
        m_dbusUpdaterProxy = new org::deepin::dde::lastore1::Updater(m_dbusName, m_dbusPath, QDBusConnection::systemBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        // 目前的Proxy封装，对于同PATH下不同Interface没有处理，且因为PATH独占，不能同时代理Manager和Updater；因此暂时在Manager里面加上对Updater的代理。
        QString updaterProxyInterface;
        if (m_proxyDbusName == "com.deepin.lastore") {
            updaterProxyInterface = "com.deepin.lastore.Updater";
        } else if (m_proxyDbusName == "org.deepin.lastore1") {
            updaterProxyInterface = "org.deepin.lastore1.Updater";
        }
        QStringList updaterFilterProps{"UpdatablePackages", "UpdatableApps"};
        connect(m_dbusUpdaterProxy, &DDBusExtendedAbstractInterface::propertyChanged, this, [=](const QString &propName, const QVariant &value){
            if (!updaterFilterProps.contains(propName)) {
                qInfo() << updaterProxyInterface << "propertyChanged-filter:" << propName << "is not allowed.";
                return;
            }
            QDBusMessage msg = QDBusMessage::createSignal(m_proxyDbusPath, "org.freedesktop.DBus.Properties", "PropertiesChanged");
            QList<QVariant> arguments;
            arguments.push_back(updaterProxyInterface);
            QVariantMap changedProps;
            changedProps.insert(propName, value);
            arguments.push_back(changedProps);
            arguments.push_back(QStringList());
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).send(msg);
        });

        SubPathInit("JobList", DBusProxySubPathInfo{"/org/deepin/dde/Lastore1/",
                                                    "com.deepin.lastore.Job",
                                                    "org.deepin.dde.Lastore1.Job"},
                    [=](QString path, QString interface, QString proxyPath, QString proxyInterface) {
                        return new SystemLastore1JobProxy(m_dbusName, path, interface,
                                                          m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
                    }
        );
    }
private:
    org::deepin::dde::lastore1::Manager *m_dbusProxy;
    org::deepin::dde::lastore1::Updater *m_dbusUpdaterProxy;
};

