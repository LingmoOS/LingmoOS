/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "key.h"

#include <KLocalizedString>
#include <krandom.h>

#include <QSharedData>

using namespace KContacts;

class Q_DECL_HIDDEN Key::Private : public QSharedData
{
public:
    Private()
        : mId(KRandom::randomString(8))
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mId = other.mId;
        mBinaryData = other.mBinaryData;
        mTextData = other.mTextData;
        mCustomTypeString = other.mCustomTypeString;
        mIsBinary = other.mIsBinary;
        mType = other.mType;
    }

    QString mId;
    QByteArray mBinaryData;
    QString mTextData;
    QString mCustomTypeString;

    Type mType;
    bool mIsBinary;
};

Key::Key(const QString &text, Type type)
    : d(new Private)
{
    d->mTextData = text;
    d->mIsBinary = false;
    d->mType = type;
}

Key::Key(const Key &other)
    : d(other.d)
{
}

Key::~Key()
{
}

bool Key::operator==(const Key &other) const
{
    if (d->mId != other.d->mId) {
        return false;
    }

    if (d->mType != other.d->mType) {
        return false;
    }

    if (d->mIsBinary != other.d->mIsBinary) {
        return false;
    }

    if (d->mIsBinary) {
        if (d->mBinaryData != other.d->mBinaryData) {
            return false;
        }
    } else {
        if (d->mTextData != other.d->mTextData) {
            return false;
        }
    }

    if (d->mCustomTypeString != other.d->mCustomTypeString) {
        return false;
    }

    return true;
}

bool Key::operator!=(const Key &other) const
{
    return !(*this == other);
}

Key &Key::operator=(const Key &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

void Key::setId(const QString &id)
{
    d->mId = id;
}

QString Key::id() const
{
    return d->mId;
}

void Key::setBinaryData(const QByteArray &binary)
{
    d->mBinaryData = binary;
    d->mIsBinary = true;
}

QByteArray Key::binaryData() const
{
    return d->mBinaryData;
}

void Key::setTextData(const QString &text)
{
    d->mTextData = text;
    d->mIsBinary = false;
}

QString Key::textData() const
{
    return d->mTextData;
}

bool Key::isBinary() const
{
    return d->mIsBinary;
}

void Key::setType(Type type)
{
    d->mType = type;
}

void Key::setCustomTypeString(const QString &custom)
{
    d->mCustomTypeString = custom;
}

Key::Type Key::type() const
{
    return d->mType;
}

QString Key::customTypeString() const
{
    return d->mCustomTypeString;
}

QString Key::toString() const
{
    QString str = QLatin1String("Key {\n");
    str += QStringLiteral("  Id: %1\n").arg(d->mId);
    str += QStringLiteral("  Type: %1\n").arg(typeLabel(d->mType));
    if (d->mType == Custom) {
        str += QStringLiteral("  CustomType: %1\n").arg(d->mCustomTypeString);
    }
    str += QStringLiteral("  IsBinary: %1\n").arg(d->mIsBinary ? QStringLiteral("true") : QStringLiteral("false"));
    if (d->mIsBinary) {
        str += QStringLiteral("  Binary: %1\n").arg(QString::fromLatin1(d->mBinaryData.toBase64()));
    } else {
        str += QStringLiteral("  Text: %1\n").arg(d->mTextData);
    }
    str += QLatin1String("}\n");

    return str;
}

Key::TypeList Key::typeList()
{
    static TypeList list;

    if (list.isEmpty()) {
        list << X509 << PGP << Custom;
    }

    return list;
}

QString Key::typeLabel(Type type)
{
    switch (type) {
    case X509:
        return i18nc("X.509 public key", "X509");
        break;
    case PGP:
        return i18nc("Pretty Good Privacy key", "PGP");
        break;
    case Custom:
        return i18nc("A custom key", "Custom");
        break;
    default:
        return i18nc("another type of encryption key", "Unknown type");
        break;
    }
}

// clang-format off
QDataStream &KContacts::operator<<(QDataStream &s, const Key &key)
{
    return s << key.d->mId << key.d->mType << key.d->mIsBinary << key.d->mBinaryData
             << key.d->mTextData << key.d->mCustomTypeString;
}

QDataStream &KContacts::operator>>(QDataStream &s, Key &key)
{
    uint type;
    s >> key.d->mId >> type >> key.d->mIsBinary >> key.d->mBinaryData >> key.d->mTextData
    >> key.d->mCustomTypeString;

    key.d->mType = Key::Type(type);

    return s;
}
// clang-format on
