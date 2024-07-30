/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2015 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETCBC_H
#define KWALLETCBC_H

#include <QObject>

class KWalletCBCTest : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void encryptDecryptOneBlock();
    void encryptDecryptMultiblock();
};

#endif // KWALLETCBC_H
