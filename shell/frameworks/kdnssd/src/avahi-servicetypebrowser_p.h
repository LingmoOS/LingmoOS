/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004,2007 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHI_SERVICETYPEBROWSER_P_H
#define AVAHI_SERVICETYPEBROWSER_P_H

#include "avahi_listener_p.h"
#include "avahi_servicetypebrowser_interface.h"
#include "servicetypebrowser.h"
#include <QStringList>
#include <QTimer>

namespace KDNSSD
{
class ServiceTypeBrowserPrivate : public QObject, public AvahiListener
{
    Q_OBJECT
public:
    ServiceTypeBrowserPrivate(ServiceTypeBrowser *parent)
        : m_browser(nullptr)
        , m_parent(parent)
        , m_started(false)
    {
    }
    ~ServiceTypeBrowserPrivate() override
    {
        if (m_browser) {
            m_browser->Free();
        }
    }

    org::freedesktop::Avahi::ServiceTypeBrowser *m_browser = nullptr;
    ServiceTypeBrowser *m_parent = nullptr;
    bool m_started = false;
    QStringList m_servicetypes;
    QString m_domain;
    QTimer m_timer;

private Q_SLOTS:
    // NB: The global slots are runtime connected! If their signature changes
    // make sure the SLOT() signature gets updated!
    void gotGlobalItemNew(int interface, int protocol, const QString &type, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalItemRemove(int interface, int protocol, const QString &type, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalAllForNow(QDBusMessage msg);

    void gotNewServiceType(int, int, const QString &, const QString &, uint);
    void gotRemoveServiceType(int, int, const QString &, const QString &, uint);
    void finished();
};

}
#endif
