/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHI_DOMAINBROWSER_P_H
#define AVAHI_DOMAINBROWSER_P_H

#include "avahi_domainbrowser_interface.h"
#include "avahi_listener_p.h"
#include "domainbrowser.h"
#include <QSet>

namespace KDNSSD
{
class DomainBrowserPrivate : public QObject, public AvahiListener
{
    Q_OBJECT
public:
    DomainBrowserPrivate(DomainBrowser::DomainType type, DomainBrowser *parent)
        : m_type(type)
        , m_browser(nullptr)
        , m_parent(parent)
        , m_started(false)
    {
    }
    ~DomainBrowserPrivate() override
    {
        if (m_browser) {
            m_browser->Free();
        }
    }

    DomainBrowser::DomainType m_type;
    org::freedesktop::Avahi::DomainBrowser *m_browser = nullptr;
    DomainBrowser *m_parent = nullptr;
    bool m_started = false;
    QSet<QString> m_domains;

public Q_SLOTS:
    // NB: The global slots are runtime connected! If their signature changes
    // make sure the SLOT() signature gets updated!
    void gotGlobalItemNew(int interface, int protocol, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalItemRemove(int interface, int protocol, const QString &domain, uint flags, QDBusMessage msg);
    void gotGlobalAllForNow(QDBusMessage msg);

    void gotNewDomain(int, int, const QString &, uint);
    void gotRemoveDomain(int, int, const QString &, uint);
};

}
#endif
