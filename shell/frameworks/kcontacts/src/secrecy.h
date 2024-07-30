/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_SECRECY_H
#define KCONTACTS_SECRECY_H

#include "kcontacts_export.h"
#include <QList>
#include <QSharedDataPointer>

namespace KContacts
{
/** Describes the confidentiality of an addressee. */
class KCONTACTS_EXPORT Secrecy
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Secrecy &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Secrecy &);

public:
    /**
     * Secrecy types
     *
     * @li Public       - for public access
     * @li Private      - only private access
     * @li Confidential - access for confidential persons
     */
    enum Type {
        Public,
        Private,
        Confidential,
        Invalid,
    };

    /**
     * List of secrecy types.
     */
    typedef QList<Type> TypeList;

    /**
     * Creates a new secrecy of the given type.
     *
     * @param type  The secrecy type. @see Type
     */
    Secrecy(Type type = Invalid);

    /**
     * Copy constructor.
     */
    Secrecy(const Secrecy &other);

    /**
     * Destroys the secrecy.
     */
    ~Secrecy();

    Secrecy &operator=(const Secrecy &other);

    Q_REQUIRED_RESULT bool operator==(const Secrecy &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Secrecy &other) const;

    /**
     * Returns if the Secrecy object has a valid value.
     */
    Q_REQUIRED_RESULT bool isValid() const;

    /**
     * Sets the @p type.
     *
     * @param type The #Type of secrecy
     */
    void setType(Type type);

    /**
     * Returns the type.
     */
    Q_REQUIRED_RESULT Type type() const;

    /**
     * Returns a list of all available secrecy types.
     */
    Q_REQUIRED_RESULT static TypeList typeList();

    /**
     * Returns a translated label for a given secrecy @p type.
     */
    Q_REQUIRED_RESULT static QString typeLabel(Type type);

    /**
     * Returns a string representation of the secrecy.
     */
    Q_REQUIRED_RESULT QString toString() const;

private:
    class PrivateData;
    QSharedDataPointer<PrivateData> d;
};

/**
 * Serializes the @p secrecy object into the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Secrecy &secrecy);

/**
 * Initializes the @p secrecy object from the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Secrecy &secrecy);
}
Q_DECLARE_TYPEINFO(KContacts::Secrecy, Q_RELOCATABLE_TYPE);
#endif
