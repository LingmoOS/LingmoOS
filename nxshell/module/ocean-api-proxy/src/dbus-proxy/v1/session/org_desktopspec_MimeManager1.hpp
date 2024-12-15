// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "session_org_desktopspec_MimeManager1.h"

class SessionMimeManager1Proxy : public DBusProxyBase {
public:
    SessionMimeManager1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr) 
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        InitFilterMethods({"listApplications", "queryDefaultApplication", "setDefaultApplication", "unsetDefaultApplication"});
        ServiceStart();
    }

    virtual	bool handleMessageCustom(const QDBusMessage &message, const QDBusConnection &connection)
    {
        if (message.interface() != m_proxyDbusInterface) {
            return false;

        }

        if (message.member() == "GetDefaultApp") {
            auto args = message.arguments();
            if (args.size() != 1) {
                return false;
            }

            auto mimetype = args.first();

            auto call = m_dbusProxy->asyncCallWithArgumentList("queryDefaultApplication", { mimetype });
            call.waitForFinished();

            auto path = call.reply().arguments().last().value<QDBusObjectPath>();
            auto appId = getAppId(path);
            auto desktop = appId + ".desktop";

            connection.send(message.createReply(QVariant::fromValue(desktop)));

            return true;
        }

        if (message.member() == "SetDefaultApp") {
            auto args = message.arguments();
            if (args.size() != 2) {
                return false;
            }

            auto mimetypes = args.first().toStringList();
            auto desktop = args.last().toString();
            auto appId = QFileInfo(desktop).completeBaseName();

            QMap<QString, QString> map;
            for (const auto &mimetype : mimetypes) {
                map.insert(mimetype, appId);
            }

            QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList("setDefaultApplication", { QVariant::fromValue(map) });
            call.waitForFinished();
            connection.send(message.createReply(call.reply().arguments()));
            return true;
        }

        return false;
    }

    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::desktopspec::MimeManager1(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }

private:
    QString getAppId(const QDBusObjectPath &appPath)
    {
        QDBusInterface interface(m_dbusName,
                                 appPath.path(),
                                 "org.desktopspec.ApplicationManager1.Application",
                                 QDBusConnection::sessionBus(),
                                 this);
        return interface.property("ID").toString();
    }

private:
    org::desktopspec::MimeManager1 *m_dbusProxy;
};


