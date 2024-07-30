/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DEFAULTCONTACTMONITOR_H
#define DEFAULTCONTACTMONITOR_H

#include <QSharedPointer>

#include "backends/allcontactsmonitor.h"
#include "backends/contactmonitor.h"

using namespace KPeople;

/*
 * If plugins don't implement a ContactWatcher, we repurpose the whole model, and single out changes for one contact
 * ideally plugins (especially slow ones) will implement their own contact monitor.
 */

class KPEOPLEBACKEND_EXPORT DefaultContactMonitor : public ContactMonitor
{
    Q_OBJECT
public:
    DefaultContactMonitor(const QString &contactUri, const AllContactsMonitorPtr &allContactsWatcher);
private Q_SLOTS:
    void onContactAdded(const QString &contactUri, const AbstractContact::Ptr &contact);
    void onContactChanged(const QString &contactUri, const AbstractContact::Ptr &contact);
    void onContactRemoved(const QString &contactUri);

private:
    AllContactsMonitorPtr m_allContactsMonitor;
};

#endif // DEFAULTCONTACTMONITOR_H
