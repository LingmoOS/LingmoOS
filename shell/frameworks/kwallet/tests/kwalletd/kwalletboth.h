/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2014 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETBOTH_H
#define KWALLETBOTH_H

#include <QObject>

class KWalletBothTest : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:
    void init();
    void openWallet();
};

#endif // KWALLETBOTH_H
