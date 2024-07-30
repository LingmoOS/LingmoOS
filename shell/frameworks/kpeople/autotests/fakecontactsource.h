/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FAKECONTACTSOURCE_H
#define FAKECONTACTSOURCE_H

#include <backends/allcontactsmonitor.h>
#include <backends/basepersonsdatasource.h>

/**
the fake datasource is configured with

ID       - NAME        - email                  - phone
contact1 - Contact1    - contact1@example.com   -
contact2 - PersonA     - contact2@example.com   - +1 234 567 890
contact3 - PersonA     - contact3@example.com   - +34 666 777 999
contact4 - PersonA     - contact4@example.com
*/

class FakeContactSource : public KPeople::BasePersonsDataSource
{
public:
    FakeContactSource(QObject *parent, const QVariantList &args = QVariantList());
    QString sourcePluginId() const override;

    void remove(const QString &uri);
    void changeProperty(const QString &key, const QVariant &value);

protected:
    KPeople::AllContactsMonitor *createAllContactsMonitor() override;
};

//----------------------------------------------------------------------------

class FakeAllContactsMonitor : public KPeople::AllContactsMonitor
{
    Q_OBJECT
public:
    explicit FakeAllContactsMonitor();
    void changeProperty(const QString &key, const QVariant &value);
    QMap<QString, KPeople::AbstractContact::Ptr> contacts() override;

    void remove(const QString &uri);

private:
    QMap<QString, KPeople::AbstractContact::Ptr> m_contacts;
};

#endif // FAKECONTACTSOURCE_H
