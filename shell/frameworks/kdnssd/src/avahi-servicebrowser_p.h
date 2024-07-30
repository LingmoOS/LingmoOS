/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHI_SERVICEBROWSER_P_H
#define AVAHI_SERVICEBROWSER_P_H

#include "avahi_listener_p.h"
#include "avahi_servicebrowser_interface.h"
#include "servicebrowser.h"
#include <QList>
#include <QString>
#include <QTimer>

namespace KDNSSD
{
class ServiceBrowserPrivate : public QObject, public AvahiListener
{
    Q_OBJECT
    friend class ServiceBrowser; // So the public class may functor connect.
public:
    ServiceBrowserPrivate(ServiceBrowser *parent)
        : QObject()
        , m_running(false)
        , m_browser(nullptr)
        , m_parent(parent)
    {
    }
    ~ServiceBrowserPrivate() override
    {
        if (m_browser) {
            m_browser->Free();
        }
        delete m_browser;
    }
    QList<RemoteService::Ptr> m_services;
    QList<RemoteService::Ptr> m_duringResolve;
    QString m_type;
    QString m_domain;
    QString m_subtype;
    bool m_autoResolve = false;
    bool m_running = false;
    bool m_finished = false;
    bool m_browserFinished = false;
    QTimer m_timer;
    org::freedesktop::Avahi::ServiceBrowser *m_browser = nullptr;
    ServiceBrowser *m_parent = nullptr;

    // get already found service identical to s or null if not found
    RemoteService::Ptr find(RemoteService::Ptr s, const QList<RemoteService::Ptr> &where) const;

private Q_SLOTS:
    void browserFinished();
    void queryFinished();
    void serviceResolved(bool success);

    // NB: The global slots are runtime connected! If their signature changes
    // make sure the SLOT() signature gets updated!
    void gotGlobalItemNew(int interface, int protocol, const QString &name, const QString &type, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalItemRemove(int interface, int protocol, const QString &name, const QString &type, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalAllForNow(QDBusMessage msg);

    void gotNewService(int, int, const QString &, const QString &, const QString &, uint);
    void gotRemoveService(int, int, const QString &, const QString &, const QString &, uint);
};

}

#endif
