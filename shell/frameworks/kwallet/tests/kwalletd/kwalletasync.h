/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2014 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETASYNC_H
#define KWALLETASYNC_H

#include <QObject>

class KWalletAsyncTest : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:
    void init();
    void openWallet();
};

#endif // KWALLETASYNC_H
