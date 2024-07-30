/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FDOSECRETSTEST_H
#define FDOSECRETSTEST_H

#include "../kwalletd.h"
#include "testhelpers.hpp"

class FdoSecretsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();

    void serviceStaticFunctions();
    void collectionStaticFunctions();

    void precreatedWallets();
    void aliases();
    void createLockUnlockCollection();
    void items();
    void session();
    void attributes();
    void walletNameEncodeDecode();
};

#endif
