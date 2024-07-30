/*
    Abstract class to load a monitor changes for a single contact
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "defaultcontactmonitor_p.h"

DefaultContactMonitor::DefaultContactMonitor(const QString &contactUri, const AllContactsMonitorPtr &allContactsWatcher)
    : ContactMonitor(contactUri)
    , m_allContactsMonitor(allContactsWatcher)
{
    connect(allContactsWatcher.data(), &AllContactsMonitor::contactAdded, this, &DefaultContactMonitor::onContactAdded);
    connect(allContactsWatcher.data(), &AllContactsMonitor::contactRemoved, this, &DefaultContactMonitor::onContactRemoved);
    connect(allContactsWatcher.data(), &AllContactsMonitor::contactChanged, this, &DefaultContactMonitor::onContactChanged);

    const QMap<QString, AbstractContact::Ptr> &contacts = m_allContactsMonitor->contacts();
    QMap<QString, AbstractContact::Ptr>::const_iterator it = contacts.constFind(contactUri);
    if (it != contacts.constEnd()) {
        setContact(it.value());
    }
}

void DefaultContactMonitor::onContactAdded(const QString &id, const AbstractContact::Ptr &contact)
{
    if (id == contactUri()) {
        setContact(contact);
    }
}

void DefaultContactMonitor::onContactChanged(const QString &id, const AbstractContact::Ptr &contact)
{
    if (id == contactUri()) {
        setContact(contact);
    }
}

void DefaultContactMonitor::onContactRemoved(const QString &id)
{
    if (id == contactUri()) {
        setContact(AbstractContact::Ptr());
    }
}

#include "moc_defaultcontactmonitor_p.cpp"
