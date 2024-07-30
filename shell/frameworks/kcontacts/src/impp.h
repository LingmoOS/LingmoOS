/*
    This file is part of libkabc.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPP_H
#define IMPP_H

#include "kcontacts_export.h"

#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

class ImppTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a IMPP for a contact.
 *
 *  IMPP stands for "Instant Messaging and Presence Protocol". This field is defined
 *  in the vCard 3.0 extension RFC 4770 and is part of vCard 4.0 (RFC 6350).
 *
 *  @since 4.14.5
 */
class KCONTACTS_EXPORT Impp
{
    Q_GADGET
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(QUrl address READ address WRITE setAddress)
    Q_PROPERTY(bool isPreferred READ isPreferred WRITE setPreferred)
    Q_PROPERTY(QString serviceType READ serviceType)
    Q_PROPERTY(QString serviceLabel READ serviceLabel)
    Q_PROPERTY(QString serviceIcon READ serviceIcon)

    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Impp &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Impp &);
    friend class VCardTool;
    friend class ::ImppTest;

public:
    Impp();
    Impp(const Impp &other);
    Impp(const QUrl &address);

    ~Impp();

    typedef QList<Impp> List;
    Q_REQUIRED_RESULT bool isValid() const;

    void setAddress(const QUrl &address);
    Q_REQUIRED_RESULT QUrl address() const;

    /**
     * Returns the messaging service this address is for.
     * This is equivalent to address().scheme().
     * @since 5.12
     */
    QString serviceType() const;
    /**
     * Returns a translated label for the service type.
     * @since 5.12
     */
    QString serviceLabel() const;
    /**
     * Returns the name of an icon representing the service type.
     * @since 5.12
     */
    QString serviceIcon() const;

    /**
     * Returns whether this is the preferred messaging address.
     * @since 5.12
     */
    bool isPreferred() const;
    /**
     * Sets that this is the preferred messaging address.
     * @since 5.12
     */
    void setPreferred(bool preferred);

    Q_REQUIRED_RESULT bool operator==(const Impp &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Impp &other) const;

    Impp &operator=(const Impp &other);

    Q_REQUIRED_RESULT QString toString() const;

    /**
     * Returns a translated name of the given IM service.
     * @since 5.12
     */
    static QString serviceLabel(const QString &serviceType);
    /**
     * Returns an icon name representing the given IM service.
     * @since 5.12
     */
    static QString serviceIcon(const QString &serviceType);
    /**
     * List all known service types.
     * Don't use these strings directly for display, instead use serviceLabel and serviceIcon.
     * @since 5.12
     */
    static QList<QString> serviceTypes();

private:
    // exported for ImppTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};

KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Impp &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Impp &object);
}

Q_DECLARE_TYPEINFO(KContacts::Impp, Q_RELOCATABLE_TYPE);

#endif // IMPP_H
