/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2014 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETPATH_H
#define KWALLETPATH_H

#include <QObject>

class KWalletPathTest : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:
    void init();
    void openWallet();
};

#endif // KWALLETPATH_H
