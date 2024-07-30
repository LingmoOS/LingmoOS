/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "global.h"
#include "kpeople_debug.h"
#include "metacontact_p.h"

#include <QSharedData>

namespace KPeople
{
class MetaContactData : public QSharedData
{
public:
    QString personUri;
    QStringList contactUris;
    AbstractContact::List contacts;
    AbstractContact::Ptr personAddressee;
};
}

// TODO: It feels like MetaContact and MetaContactProxy should be merged,
// still, not today.

class MetaContactProxy : public KPeople::AbstractContact
{
public:
    MetaContactProxy(const AbstractContact::List &contacts)
        : m_contacts(contacts)
    {
    }

    QVariant customProperty(const QString &key) const override
    {
        if (key.startsWith(QLatin1String("all-"))) {
            QVariantList ret;
            for (const AbstractContact::Ptr &contact : std::as_const(m_contacts)) {
                QVariant val = contact->customProperty(key);
                Q_ASSERT(val.canConvert<QVariantList>() || val.isNull());

                if (!val.isNull()) {
                    ret.append(val.toList());
                }
            }
            return ret;
        } else {
            for (const AbstractContact::Ptr &contact : std::as_const(m_contacts)) {
                QVariant val = contact->customProperty(key);
                if (val.isValid()) {
                    return val;
                }
            }
            return QVariant();
        }
    }

    const AbstractContact::List m_contacts;
};

using namespace KPeople;

MetaContact::MetaContact()
    : d(new MetaContactData)
{
    reload();
}

MetaContact::MetaContact(const QString &personUri, const QMap<QString, AbstractContact::Ptr> &contacts)
    : d(new MetaContactData)
{
    d->personUri = personUri;

    QMap<QString, AbstractContact::Ptr>::const_iterator it = contacts.constBegin();
    while (it != contacts.constEnd()) {
        insertContactInternal(it.key(), it.value());
        it++;
    }
    reload();
}

MetaContact::MetaContact(const QString &contactUri, const AbstractContact::Ptr &contact)
    : d(new MetaContactData)
{
    d->personUri = contactUri;
    insertContactInternal(contactUri, contact);
    reload();
}

MetaContact::MetaContact(const MetaContact &other)
    : d(other.d)
{
}

MetaContact &MetaContact::operator=(const MetaContact &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

MetaContact::~MetaContact()
{
}

QString MetaContact::id() const
{
    return d->personUri;
}

bool MetaContact::isValid() const
{
    return !d->contacts.isEmpty();
}

QStringList MetaContact::contactUris() const
{
    return d->contactUris;
}

AbstractContact::Ptr MetaContact::contact(const QString &contactUri)
{
    int index = d->contactUris.indexOf(contactUri);
    if (index >= 0) {
        return d->contacts[index];
    } else {
        return AbstractContact::Ptr();
    }
}

AbstractContact::List MetaContact::contacts() const
{
    return d->contacts;
}

const AbstractContact::Ptr &MetaContact::personAddressee() const
{
    Q_ASSERT(d->personAddressee);
    return d->personAddressee;
}

int MetaContact::insertContact(const QString &contactUri, const AbstractContact::Ptr &contact)
{
    int index = insertContactInternal(contactUri, contact);
    if (index >= 0) {
        reload();
    } else {
        qCWarning(KPEOPLE_LOG) << "Inserting an already-present contact" << contactUri;
    }
    return index;
}

int MetaContact::insertContactInternal(const QString &contactUri, const AbstractContact::Ptr &contact)
{
    if (d->contactUris.contains(contactUri)) {
        // if item is already listed, do nothing.
        return -1;
    } else {
        // TODO if from the local address book - prepend to give higher priority.
        int index = d->contacts.size();
        d->contacts.append(contact);
        d->contactUris.append(contactUri);
        return index;
    }
}

int MetaContact::updateContact(const QString &contactUri, const AbstractContact::Ptr &contact)
{
    const int index = d->contactUris.indexOf(contactUri);
    Q_ASSERT(index < 0 || d->contacts[index] == contact);
    if (index < 0) {
        qCWarning(KPEOPLE_LOG) << "contact not part of the metacontact";
    }
    return index;
}

int MetaContact::removeContact(const QString &contactUri)
{
    const int index = d->contactUris.indexOf(contactUri);
    if (index >= 0) {
        d->contacts.removeAt(index);
        d->contactUris.removeAt(index);
        reload();
    }
    return index;
}

void MetaContact::reload()
{
    // always favour the first item

    // Optimization, if only one contact re-use that one
    d->personAddressee = (d->contacts.size() == 1) ? d->contacts.first() : AbstractContact::Ptr(new MetaContactProxy(d->contacts));
    Q_ASSERT(d->personAddressee);
}
