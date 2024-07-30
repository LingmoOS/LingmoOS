/*
    SPDX-FileCopyrightText: 2014 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPEOPLE_ABSTRACT_CONTACT_H
#define KPEOPLE_ABSTRACT_CONTACT_H

#include <QSharedData>
#include <QVariant>
#include <kpeoplebackend/kpeoplebackend_export.h>

namespace KPeople
{
/**
 * @brief KPeople::AbstractContact is the class to provide the data from a given
 * contact by the backends.
 *
 * To obtain it from a front-end application PersonData and PersonsModel
 * should be used.
 *
 * @since 5.8
 * @internal
 */

class KPEOPLEBACKEND_EXPORT AbstractContact : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<AbstractContact> Ptr;
    typedef QList<AbstractContact::Ptr> List;
    AbstractContact();
    virtual ~AbstractContact();

    //     well-known properties
    /** String property representing the display name of the contact */
    static const QString NameProperty;

    /** String property representing the preferred name of the contact */
    static const QString EmailProperty;

    /** String property representing the preferred phone number of the contact */
    static const QString PhoneNumberProperty;

    /** QVariantList property that lists all phone numbers the contact has */
    static const QString AllPhoneNumbersProperty;

    /**
     * String property representing the IM presence of the contact.
     * @sa KPeople::iconNameForPresenceString()
     */
    static const QString PresenceProperty;

    /**
     * QUrl or QPixmap property representing the contacts' avatar
     */
    static const QString PictureProperty;

    /** QVariantList property that lists the groups the contacts belongs to */
    static const QString GroupsProperty;

    /** QVariantList property that lists the emails the contact has */
    static const QString AllEmailsProperty;

    /** QByteArray with the raw vcard information */
    static const QString VCardProperty;

    /**
     * Generic method to access a random contact property
     *
     * @returns the value for the @p key property.
     */
    virtual QVariant customProperty(const QString &key) const = 0;

private:
    Q_DISABLE_COPY(AbstractContact)
};

}

Q_DECLARE_METATYPE(KPeople::AbstractContact::List)
Q_DECLARE_METATYPE(KPeople::AbstractContact::Ptr)

#endif
