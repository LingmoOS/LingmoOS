/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "allcontactsmonitor.h"

using namespace KPeople;

class KPeople::AllContactsMonitorPrivate
{
public:
    AllContactsMonitorPrivate()
    {
    }

    bool m_initialFetchDone = false;
    bool m_initialFetchSucccess = false;
};

AllContactsMonitor::AllContactsMonitor()
    : QObject()
    , d_ptr(new AllContactsMonitorPrivate)
{
}
AllContactsMonitor::~AllContactsMonitor()
{
    delete d_ptr;
}

QMap<QString, AbstractContact::Ptr> AllContactsMonitor::contacts()
{
    return QMap<QString, AbstractContact::Ptr>();
}

bool AllContactsMonitor::isInitialFetchComplete() const
{
    return d_ptr->m_initialFetchDone;
}

bool AllContactsMonitor::initialFetchSuccess() const
{
    return d_ptr->m_initialFetchSucccess;
}

void AllContactsMonitor::emitInitialFetchComplete(bool success)
{
    d_ptr->m_initialFetchDone = true;
    d_ptr->m_initialFetchSucccess = success;
    Q_EMIT initialFetchComplete(success);
}

#include "moc_allcontactsmonitor.cpp"
