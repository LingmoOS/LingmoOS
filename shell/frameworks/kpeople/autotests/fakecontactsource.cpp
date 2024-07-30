/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fakecontactsource.h"

FakeContactSource::FakeContactSource(QObject *parent, const QVariantList &args)
    : BasePersonsDataSource(parent, args)
{
}

QString FakeContactSource::sourcePluginId() const
{
    return QStringLiteral("fakesource://");
}

KPeople::AllContactsMonitor *FakeContactSource::createAllContactsMonitor()
{
    return new FakeAllContactsMonitor();
}

void FakeContactSource::changeProperty(const QString &key, const QVariant &value)
{
    qobject_cast<FakeAllContactsMonitor *>(allContactsMonitor().data())->changeProperty(key, value);
}

void FakeContactSource::remove(const QString &uri)
{
    qobject_cast<FakeAllContactsMonitor *>(allContactsMonitor().data())->remove(uri);
}

//----------------------------------------------------------------------------

class FakeContact : public KPeople::AbstractContact
{
public:
    FakeContact(const QVariantMap &props)
        : m_properties(props)
    {
    }

    QVariant customProperty(const QString &key) const override
    {
        if (key.startsWith(QLatin1String("all-"))) {
            return QStringList(m_properties[key.mid(4)].toString());
        } else {
            return m_properties[key];
        }
    }

    QVariantMap m_properties;
};

FakeAllContactsMonitor::FakeAllContactsMonitor()
    : m_contacts(
        {{QStringLiteral("fakesource://contact1"),
          KPeople::AbstractContact::Ptr(new FakeContact(QVariantMap{{KPeople::AbstractContact::NameProperty, QStringLiteral("Contact 1")},
                                                                    {KPeople::AbstractContact::EmailProperty, QStringLiteral("contact1@example.com")}}))},

         {QStringLiteral("fakesource://contact2"),
          KPeople::AbstractContact::Ptr(new FakeContact(QVariantMap{{KPeople::AbstractContact::NameProperty, QStringLiteral("Contact 2")},
                                                                    {KPeople::AbstractContact::EmailProperty, QStringLiteral("contact2@example.com")},
                                                                    {KPeople::AbstractContact::PhoneNumberProperty, QStringLiteral("+1 234 567 890")}}))},

         {QStringLiteral("fakesource://contact3"),
          KPeople::AbstractContact::Ptr(new FakeContact(QVariantMap{{KPeople::AbstractContact::NameProperty, QStringLiteral("Contact 3")},
                                                                    {KPeople::AbstractContact::EmailProperty, QStringLiteral("contact3@example.com")},
                                                                    {KPeople::AbstractContact::PhoneNumberProperty, QStringLiteral("+34 666 777 999")}}))},

         {QStringLiteral("fakesource://contact4"),
          KPeople::AbstractContact::Ptr(new FakeContact(QVariantMap{{KPeople::AbstractContact::NameProperty, QStringLiteral("Contact 4")},
                                                                    {KPeople::AbstractContact::EmailProperty, QStringLiteral("contact4@example.com")},
                                                                    {KPeople::AbstractContact::PresenceProperty, QStringLiteral("online")}}))}})
{
    emitInitialFetchComplete(true);
}

void FakeAllContactsMonitor::remove(const QString &uri)
{
    m_contacts.remove(uri);
    Q_EMIT contactRemoved(uri);
}

QMap<QString, KPeople::AbstractContact::Ptr> FakeAllContactsMonitor::contacts()
{
    return m_contacts;
}

void FakeAllContactsMonitor::changeProperty(const QString &key, const QVariant &value)
{
    KPeople::AbstractContact::Ptr contact1 = contacts()[QStringLiteral("fakesource://contact1")];
    static_cast<FakeContact *>(contact1.data())->m_properties[key] = value;
    Q_ASSERT(contact1->customProperty(key) == value);

    Q_EMIT contactChanged(QStringLiteral("fakesource://contact1"), contact1);
}

#include "moc_fakecontactsource.cpp"
