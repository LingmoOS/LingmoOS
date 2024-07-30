/*
    Abstract class to load a monitor changes for a single contact
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "contactmonitor.h"

using namespace KPeople;

class KPeople::ContactMonitorPrivate
{
public:
    QString m_contactUri;
    AbstractContact::Ptr m_contact;
};

ContactMonitor::ContactMonitor(const QString &contactUri)
    : QObject(nullptr)
    , d_ptr(new ContactMonitorPrivate)
{
    Q_D(ContactMonitor);
    d->m_contactUri = contactUri;
}

ContactMonitor::~ContactMonitor()
{
    delete d_ptr;
}

void ContactMonitor::setContact(const AbstractContact::Ptr &contact)
{
    Q_D(ContactMonitor);

    d->m_contact = contact;
    Q_EMIT contactChanged();
}

AbstractContact::Ptr ContactMonitor::contact() const
{
    Q_D(const ContactMonitor);

    return d->m_contact;
}

QString ContactMonitor::contactUri() const
{
    Q_D(const ContactMonitor);

    return d->m_contactUri;
}

#include "moc_contactmonitor.cpp"
