/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001-2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletentry.h"

using namespace KWallet;
#include <QDataStream>
#include <QIODevice>

Entry::Entry()
{
}

Entry::~Entry()
{
    _value.fill(0);
}

const QString &Entry::key() const
{
    return _key;
}

const QByteArray &Entry::value() const
{
    return _value;
}

QString Entry::password() const
{
    QString x;
    QDataStream qds(_value);
    qds >> x;
    return x;
}

void Entry::setValue(const QByteArray &val)
{
    // do a direct copy from one into the other without
    // temporary variables
    _value.fill(0);
    _value = val;
}

void Entry::setValue(const QString &val)
{
    _value.fill(0);
    QDataStream qds(&_value, QIODevice::WriteOnly);
    qds << val;
}

void Entry::setKey(const QString &key)
{
    _key = key;
}

Wallet::EntryType Entry::type() const
{
    return _type;
}

void Entry::setType(Wallet::EntryType type)
{
    _type = type;
}

void Entry::copy(const Entry *x)
{
    _type = x->_type;
    _key = x->_key;
    _value.fill(0);
    _value = x->_value;
}

