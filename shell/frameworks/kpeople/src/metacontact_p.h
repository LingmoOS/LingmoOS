/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef METACONTACT_H
#define METACONTACT_H

#include "backends/abstractcontact.h"
#include <QSharedDataPointer>

#include <kpeople/kpeople_export.h>

namespace KPeople
{
class MetaContactData;

class MetaContact
{
public:
    MetaContact();

    /** Create a 'MetaContact' from a single contact*/
    MetaContact(const QString &contactUri, const AbstractContact::Ptr &contact);

    /** Create a MetaContact with a given person ID and a map of all associated contacts*/
    MetaContact(const QString &personUri, const QMap<QString, AbstractContact::Ptr> &contacts);
    MetaContact(const MetaContact &other);
    ~MetaContact();

    MetaContact &operator=(const MetaContact &other);

    QString id() const;
    bool isValid() const;

    QStringList contactUris() const;
    AbstractContact::List contacts() const;

    AbstractContact::Ptr contact(const QString &contactUri);
    const AbstractContact::Ptr &personAddressee() const;

    // update one of the stored contacts in this metacontact object
    //@return the index of the contact which was inserted

    int insertContact(const QString &contactUri, const AbstractContact::Ptr &contact);

    int updateContact(const QString &contactUri, const AbstractContact::Ptr &contact);

    int removeContact(const QString &contactUri);

private:
    // does the real inserting contacts. Split so that we don't call the expensive "reload" function
    // multiple times at startup
    int insertContactInternal(const QString &contactUri, const AbstractContact::Ptr &contact);

    void reload();

    QSharedDataPointer<MetaContactData> d;
};
}

Q_DECLARE_TYPEINFO(KPeople::MetaContact, Q_RELOCATABLE_TYPE);

#endif // METACONTACT_H
