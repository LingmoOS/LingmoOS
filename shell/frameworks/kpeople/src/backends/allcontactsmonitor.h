/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef ALLCONTACTSMONITOR_H
#define ALLCONTACTSMONITOR_H

#include <QObject>
#include <QSharedPointer>

#include <kpeoplebackend/abstractcontact.h>
#include <kpeoplebackend/kpeoplebackend_export.h>

namespace KPeople
{
class AllContactsMonitorPrivate;

/**
 * This class should be subclassed by each datasource and return a list of
 * all contacts that the datasource knows about.
 *
 * Subclasses are expected to be asynchronous
 *
 * @since 5.8
 */
class KPEOPLEBACKEND_EXPORT AllContactsMonitor : public QObject
{
    Q_OBJECT
public:
    explicit AllContactsMonitor(); // TODO make protected? this isn't useful unless subclassed
    ~AllContactsMonitor() override;

    /**
     * Returns all currently loaded contacts
     */
    virtual QMap<QString, AbstractContact::Ptr> contacts();

    // TODO redo as a state enum - InitialLoad, Fail, Loaded
    bool isInitialFetchComplete() const;

    bool initialFetchSuccess() const;

Q_SIGNALS:
    /**
     * DataSources should emit this whenever a known contact changes
     */
    void contactChanged(const QString &contactUri, const KPeople::AbstractContact::Ptr &contact);

    /**
     * DataSources should emit this whenever a contact is added
     */
    void contactAdded(const QString &contactUri, const KPeople::AbstractContact::Ptr &contact);

    /**
     * DataSources should emit this whenever a contact is removed and they are no longer able to supply up-to-date data on a contact
     */
    void contactRemoved(const QString &contactUri);

    /**
     * Notifies that the DataSource has completed it's initial fetch.
     *
     * @warning DataSources should use emitInitialFetchComplete() instead of emitting this signal
     * directly.
     *
     * @param success True when the fetch was successful, False when an error occurred.
     */
    void initialFetchComplete(bool success);

protected Q_SLOTS:
    /**
     * DataSources should call this once they have finished initial retrieval of all contacts from their
     * storage.
     *
     * This will emit initialFetchComplete() signal
     *
     * @p success Whether the fetch was successful.
     */
    void emitInitialFetchComplete(bool success);

private:
    Q_DISABLE_COPY(AllContactsMonitor)
    Q_DECLARE_PRIVATE(AllContactsMonitor)
    AllContactsMonitorPrivate *d_ptr;
};

}
typedef QSharedPointer<KPeople::AllContactsMonitor> AllContactsMonitorPtr;

#endif // ALLCONTACTSMONITOR_H
