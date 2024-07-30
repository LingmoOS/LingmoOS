/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHI_REMOTESERVICE_P_H
#define AVAHI_REMOTESERVICE_P_H

#include "avahi_listener_p.h"
#include "avahi_serviceresolver_interface.h"
#include "remoteservice.h"
#include "servicebase_p.h"
#include <QList>
#include <QMap>
#include <QString>

#define KDNSSD_D RemoteServicePrivate *d = static_cast<RemoteServicePrivate *>(this->d.operator->())

namespace KDNSSD
{
class RemoteServicePrivate : public QObject, public ServiceBasePrivate, public AvahiListener
{
    Q_OBJECT
public:
    RemoteServicePrivate(RemoteService *parent, const QString &name, const QString &type, const QString &domain)
        : QObject()
        , ServiceBasePrivate(name, type, domain, QString(), 0)
        , m_resolved(false)
        , m_running(false)
        , m_resolver(nullptr)
        , m_parent(parent)
    {
    }
    ~RemoteServicePrivate() override
    {
        if (m_resolver) {
            m_resolver->Free();
        }
        delete m_resolver;
    }
    bool m_resolved = false;
    bool m_running = false;
    org::freedesktop::Avahi::ServiceResolver *m_resolver = nullptr;
    RemoteService *m_parent = nullptr;
    void stop();

private Q_SLOTS:
    // NB: The global slots are runtime connected! If their signature changes
    // make sure the SLOT() signature gets updated!
    void gotGlobalFound(int interface,
                        int protocol,
                        const QString &name,
                        const QString &type,
                        const QString &domain,
                        const QString &host,
                        int aprotocol,
                        const QString &address,
                        ushort port,
                        const QList<QByteArray> &txt,
                        uint flags,
                        QDBusMessage msg);
    void gotGlobalError(QDBusMessage msg);

    void gotFound(int interface,
                  int protocol,
                  const QString &name,
                  const QString &type,
                  const QString &domain,
                  const QString &host,
                  int aprotocol,
                  const QString &address,
                  ushort port,
                  const QList<QByteArray> &txt,
                  uint flags);
    void gotError();
};

}

#endif
