/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "basepersonsdatasource.h"

#include "backends/abstractcontact.h"
#include "defaultcontactmonitor_p.h"

using namespace KPeople;

class KPeople::BasePersonsDataSourcePrivate
{
public:
    QWeakPointer<AllContactsMonitor> m_allContactsMonitor;
    QHash<QString, QWeakPointer<ContactMonitor>> m_contactMonitors;
};

BasePersonsDataSource::BasePersonsDataSource(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d_ptr(new BasePersonsDataSourcePrivate)
{
    Q_UNUSED(args)
}

BasePersonsDataSource::~BasePersonsDataSource()
{
    delete d_ptr;
}

AllContactsMonitorPtr BasePersonsDataSource::allContactsMonitor()
{
    Q_D(BasePersonsDataSource);

    // if there is currently no watcher, create one
    AllContactsMonitorPtr c = d->m_allContactsMonitor.toStrongRef();
    if (!c) {
        c = AllContactsMonitorPtr(createAllContactsMonitor());
        d->m_allContactsMonitor = c;
    }

    return c;
}

ContactMonitorPtr BasePersonsDataSource::contactMonitor(const QString &contactUri)
{
    Q_D(BasePersonsDataSource);

    ContactMonitorPtr c = d->m_contactMonitors[contactUri].toStrongRef();
    if (!c) {
        c = ContactMonitorPtr(createContactMonitor(contactUri));
        d->m_contactMonitors[contactUri] = c;
    }
    return c;
}

ContactMonitor *BasePersonsDataSource::createContactMonitor(const QString &contactUri)
{
    return new DefaultContactMonitor(contactUri, allContactsMonitor());
}

BasePersonsDataSourceV2::BasePersonsDataSourceV2(QObject *parent, const QVariantList &args)
    : BasePersonsDataSource(parent, args)
{
}

#include "moc_basepersonsdatasource.cpp"
