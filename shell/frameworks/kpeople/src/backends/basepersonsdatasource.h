/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BASE_PERSONS_DATA_SOURCE_H
#define BASE_PERSONS_DATA_SOURCE_H

#include <QObject>
#include <QVariant>

#include <kpeoplebackend/kpeoplebackend_export.h>

#include "allcontactsmonitor.h"
#include "contactmonitor.h"

namespace KPeople
{
class BasePersonsDataSourcePrivate;

// This is a QObject for KPluginFactory
class KPEOPLEBACKEND_EXPORT BasePersonsDataSource : public QObject
{
    Q_OBJECT
public:
    BasePersonsDataSource(QObject *parent, const QVariantList &args = QVariantList());
    ~BasePersonsDataSource() override;

    /**
     * Return a ref counted AllContactMonitor which lists and monitors all contacts from a source
     */
    AllContactsMonitorPtr allContactsMonitor();

    /**
     * Return a ref counted watcher for a single contact
     */
    ContactMonitorPtr contactMonitor(const QString &contactUri);

    /**
     * Returns the ID used by this datasource.
     * i.e if the contactIDs are in the form akonadi://?item=324 this method should return "akonadi"
     */
    virtual QString sourcePluginId() const = 0;

protected:
    virtual AllContactsMonitor *createAllContactsMonitor() = 0;

    /**
     * Base classes can implement this in order to not load every contact
     * otherwise the AllContactWatcher will be used and filtered.
     */
    virtual ContactMonitor *createContactMonitor(const QString &contactUri);

private:
    Q_DISABLE_COPY(BasePersonsDataSource)
    Q_DECLARE_PRIVATE(BasePersonsDataSource)
    BasePersonsDataSourcePrivate *d_ptr;
};

class KPEOPLEBACKEND_EXPORT BasePersonsDataSourceV2 : public BasePersonsDataSource
{
    Q_OBJECT
public:
    BasePersonsDataSourceV2(QObject *parent, const QVariantList &args = QVariantList());

    virtual bool addContact(const QVariantMap &properties) = 0;
    virtual bool deleteContact(const QString &uri) = 0;
};

}
#endif // BASE_PERSONS_DATA_SOURCE_H
