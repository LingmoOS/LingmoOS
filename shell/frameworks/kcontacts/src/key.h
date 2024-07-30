/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_KEY_H
#define KCONTACTS_KEY_H

#include "kcontacts_export.h"

#include <QDataStream>
#include <QSharedDataPointer>

namespace KContacts
{
/**
 * @short A class to store an encryption key.
 */
class KCONTACTS_EXPORT Key
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Key &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Key &);

public:
    /**
      List of keys.
    */
    typedef QList<Key> List;

    /**
      Key types
    */
    enum Type {
        X509, /**< X509 key */
        PGP, /**< Pretty Good Privacy key */
        Custom, /**< Custom or IANA conform key */
    };

    /**
      List of key types.
    */
    typedef QList<Type> TypeList;

    /**
      Creates a new key.

      @param text  The text data.
      @param type  The key type, see Types.
    */
    explicit Key(const QString &text = QString(), Type type = PGP);

    /**
      Copy constructor.
    */
    Key(const Key &other);

    /**
      Destroys the key.
    */
    ~Key();

    /**
      Equality operator.
    */
    Q_REQUIRED_RESULT bool operator==(const Key &other) const;

    /**
      Not-equal operator.
    */
    Q_REQUIRED_RESULT bool operator!=(const Key &other) const;

    /**
      Assignment operator.

      @param other The Key instance to assign to @c this
    */
    Key &operator=(const Key &other);

    /**
      Sets the unique @p identifier.
    */
    void setId(const QString &identifier);

    /**
      Returns the unique identifier.
    */
    Q_REQUIRED_RESULT QString id() const;

    /**
      Sets binary @p data.
    */
    void setBinaryData(const QByteArray &data);

    /**
      Returns the binary data.
    */
    Q_REQUIRED_RESULT QByteArray binaryData() const;

    /**
      Sets text @p data.
    */
    void setTextData(const QString &data);

    /**
      Returns the text data.
    */
    Q_REQUIRED_RESULT QString textData() const;

    /**
      Returns whether the key contains binary or text data.
    */
    Q_REQUIRED_RESULT bool isBinary() const;

    /**
      Sets the @p type.

      @param type The type of the key

      @see Type
    */
    void setType(Type type);

    /**
      Sets custom @p type string.
    */
    void setCustomTypeString(const QString &type);

    /**
      Returns the type, see Type.
    */
    Q_REQUIRED_RESULT Type type() const;

    /**
      Returns the custom type string.
    */
    Q_REQUIRED_RESULT QString customTypeString() const;

    /**
      Returns a string representation of the key.
    */
    Q_REQUIRED_RESULT QString toString() const;

    /**
      Returns a list of all available key types.
    */
    Q_REQUIRED_RESULT static TypeList typeList();

    /**
      Returns a translated label for a given key @p type.
    */
    Q_REQUIRED_RESULT static QString typeLabel(Type type);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
  Serializes the @p key object into the @p stream.
*/
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Key &key);

/**
  Initializes the @p key object from the @p stream.
*/
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Key &key);
}
Q_DECLARE_TYPEINFO(KContacts::Key, Q_RELOCATABLE_TYPE);
#endif
