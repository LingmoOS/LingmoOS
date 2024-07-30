/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHI_PUBLICSERVICE_P_H
#define AVAHI_PUBLICSERVICE_P_H

#include "avahi_entrygroup_interface.h"
#include "avahi_listener_p.h"
#include "avahi_server_interface.h"
#include "publicservice.h"
#include "servicebase_p.h"
#include <QStringList>
#include <avahi-common/defs.h>

#define KDNSSD_D PublicServicePrivate *d = static_cast<PublicServicePrivate *>(this->d.operator->())

namespace KDNSSD
{
class PublicServicePrivate : public QObject, public ServiceBasePrivate, public AvahiListener
{
    Q_OBJECT
public:
    PublicServicePrivate(PublicService *parent, const QString &name, const QString &type, const QString &domain, unsigned int port)
        : QObject()
        , ServiceBasePrivate(name, type, domain, QString(), port)
        , m_published(false)
        , m_running(false)
        , m_group(nullptr)
        , m_server(nullptr)
        , m_collision(false)
        , m_parent(parent)
    {
    }
    ~PublicServicePrivate() override
    {
        if (m_group) {
            m_group->Free();
        }
        delete m_group;
        delete m_server;
    }

    bool m_published;
    bool m_running;
    org::freedesktop::Avahi::EntryGroup *m_group;
    org::freedesktop::Avahi::Server *m_server;
    bool m_collision;
    QStringList m_subtypes;
    PublicService *m_parent;

    void commit()
    {
        if (!m_collision) {
            m_group->Commit();
        }
    }
    void stop();
    bool fillEntryGroup();
    void tryApply();

public Q_SLOTS:
    // NB: The global slots are runtime connected! If their signature changes
    // make sure the SLOT() signature gets updated!
    void gotGlobalStateChanged(int state, const QString &error, QDBusMessage msg);

    void serverStateChanged(int, const QString &);
    void groupStateChanged(int, const QString &);
};

}

#endif
