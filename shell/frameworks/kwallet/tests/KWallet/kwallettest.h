/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2007 Thomas McGuire <thomas.mcguire@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETTEST_H
#define KWALLETTEST_H

#include <QObject>

class KWalletTest : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:
    void init();
    void testWallet();
};

#endif
