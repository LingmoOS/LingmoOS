/*
    Abstract class to load a monitor changes for a single contact
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef CONTACTMONITOR_H
#define CONTACTMONITOR_H

#include <QObject>

#include <kpeoplebackend/kpeoplebackend_export.h>

#include "abstractcontact.h"

namespace KPeople
{
class ContactMonitorPrivate;

/**
 * This class loads data for a single contact from a datasource.
 *
 * Datasources should subclass this and call setContact() when the contact loads or changes.
 * It is used for optimising performance over loading all contacts and filtering the results.
 * Subclasses are expected to be asynchronous in loading data.
 *
 * @since 5.8
 */
class KPEOPLEBACKEND_EXPORT ContactMonitor : public QObject
{
    Q_OBJECT
public:
    ContactMonitor(const QString &contactUri);
    ~ContactMonitor() override;

    /**
     * The ID of the contact being loaded
     */
    QString contactUri() const;

    /**
     * The currently loaded information on this contact.
     */
    AbstractContact::Ptr contact() const;
Q_SIGNALS:
    /**
     * Emitted whenever the contact changes
     */
    void contactChanged();

protected:
    /**
     * Sets or updates the contact and emits contactChanged
     * Subclasses should call this when data is loaded or changes
     */
    void setContact(const AbstractContact::Ptr &contact);

private:
    Q_DISABLE_COPY(ContactMonitor)
    Q_DECLARE_PRIVATE(ContactMonitor)
    ContactMonitorPrivate *d_ptr;
};

}
typedef QSharedPointer<KPeople::ContactMonitor> ContactMonitorPtr;

#endif // CONTACTMONITOR_H
