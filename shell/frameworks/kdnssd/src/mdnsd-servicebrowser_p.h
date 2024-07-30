/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MDNSD_SERVICEBROWSER_P_H
#define MDNSD_SERVICEBROWSER_P_H

#include <QObject>
#include <QTimer>

#include "mdnsd-responder.h"
#include "servicebrowser.h"

namespace KDNSSD
{
class ServiceBrowserPrivate : public Responder
{
    Q_OBJECT
public:
    ServiceBrowserPrivate(ServiceBrowser *parent)
        : Responder()
        , m_parent(parent)
    {
    }
    QList<RemoteService::Ptr> m_services;
    QList<RemoteService::Ptr> m_duringResolve;
    QString m_type;
    QString m_domain;
    QString m_subtype;
    bool m_autoResolve;
    bool m_finished;
    ServiceBrowser *m_parent;
    QTimer timeout;

    // get already found service identical to s or null if not found
    RemoteService::Ptr find(RemoteService::Ptr s, const QList<RemoteService::Ptr> &where) const;
    virtual void customEvent(QEvent *event);
public Q_SLOTS:
    void queryFinished();
    void serviceResolved(bool success);
    void onTimeout();
};

}

#endif
