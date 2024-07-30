/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "accountbalance.h"

using namespace Attica;

class Q_DECL_HIDDEN AccountBalance::Private : public QSharedData
{
public:
    QString balance;
    QString currency;
};

AccountBalance::AccountBalance()
    : d(new Private)
{
}

AccountBalance::AccountBalance(const Attica::AccountBalance &other)
    : d(other.d)
{
}

AccountBalance &AccountBalance::operator=(const Attica::AccountBalance &other)
{
    d = other.d;
    return *this;
}

AccountBalance::~AccountBalance()
{
}

void AccountBalance::setBalance(const QString &balance)
{
    d->balance = balance;
}

QString AccountBalance::balance() const
{
    return d->balance;
}

void AccountBalance::setCurrency(const QString &currency)
{
    d->currency = currency;
}

QString AccountBalance::currency() const
{
    return d->currency;
}
