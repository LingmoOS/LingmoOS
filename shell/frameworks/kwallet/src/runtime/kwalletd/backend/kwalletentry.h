/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001-2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KWALLETENTRY_H
#define _KWALLETENTRY_H

#include <QString>

#include <kwallet.h>

#include "kwalletbackend_export.h"

namespace KWallet
{
/* @internal
 */
class KWALLETBACKEND_EXPORT Entry
{
public:
    Entry();
    ~Entry();

    const QString &key() const;
    const QByteArray &value() const;
    QString password() const;
    const QByteArray &map() const
    {
        return value();
    }

    void setValue(const QByteArray &val);
    void setValue(const QString &val);
    void setKey(const QString &key);

    Wallet::EntryType type() const;
    void setType(Wallet::EntryType type);

    void copy(const Entry *x);

private:
    QString _key;
    QByteArray _value;
    Wallet::EntryType _type;
};

}

#endif
