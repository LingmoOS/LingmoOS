/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2014 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETSYNC_H
#define KWALLETSYNC_H

#include <QObject>

class KWalletSyncTest : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:
    void init();
    void openWallet();
};

#endif // KWALLETSYNC_H
