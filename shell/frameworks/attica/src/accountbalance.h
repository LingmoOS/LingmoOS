/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_ACCOUNTBALANCE_H
#define ATTICA_ACCOUNTBALANCE_H

#include <QSharedDataPointer>
#include <QString>

#include "attica_export.h"

namespace Attica
{
/**
 * @class AccountBalance accountbalance.h <Attica/AccountBalance>
 *
 * Represents the money in the account of the user
 */
class ATTICA_EXPORT AccountBalance
{
public:
    typedef QList<AccountBalance> List;
    class Parser;

    /**
     * Creates an empty AccountBalance
     */
    AccountBalance();

    /**
     * Copy constructor.
     * @param other the AccountBalance to copy from
     */
    AccountBalance(const AccountBalance &other);

    /**
     * Assignment operator.
     * @param other the AccountBalance to assign from
     * @return pointer to this AccountBalance
     */
    AccountBalance &operator=(const AccountBalance &other);

    /**
     * Destructor.
     */
    ~AccountBalance();

    /**
     * Sets the currency in use.
     * @param currency the new currency (Euro, US Dollar)
     */
    void setCurrency(const QString &currency);

    /**
     * Gets the currency.
     * @return the currency
     */
    QString currency() const;

    /**
     * Sets the balance.
     * @param balance
     */
    void setBalance(const QString &name);

    /**
     * Gets the balance.
     * @return the amount of money in the account
     */
    QString balance() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif // ACCOUNTBALANCE_H
