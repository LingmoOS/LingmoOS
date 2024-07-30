/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETMANY_H
#define KWALLETMANY_H

#include <QList>
#include <QObject>

namespace KWallet
{
class Wallet;
}

class KWalletMany : public QObject
{
    Q_OBJECT

public:
    KWalletMany();
    ~KWalletMany() override;

public Q_SLOTS:
    void walletOpened(bool open);

private Q_SLOTS:
    void init();
    void openWallet();

private:
    QList<KWallet::Wallet *> _wallets;
    int _pending;
};

#endif // KWALLETMANY_H
