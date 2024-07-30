/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2015 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletcbc.h"

#include <QTest>
#include <backend/blowfish.h>
#include <backend/cbc.h>

void KWalletCBCTest::encryptDecryptOneBlock()
{
    BlockCipher *bf;
    char data[] = "OneBlock";
    char key[] = "testkey";

    bf = new BlowFish();

    bf->setKey((void *)key, 7 * 8);

    QVERIFY(bf->readyToGo());
    QVERIFY(8 == bf->encrypt((void *)data, 8));

    delete bf;
    bf = new BlowFish();
    bf->setKey((void *)key, 7 * 8);

    QVERIFY(8 == bf->decrypt((void *)data, 8));
    QVERIFY(strcmp(data, "OneBlock") == 0);
    delete bf;
}

void KWalletCBCTest::encryptDecryptMultiblock()
{
    BlockCipher *bf;
    char data[] = "This is a test.";
    char key[] = "testkey";

    bf = new BlowFish();

    bf->setKey((void *)key, 7 * 8);

    QVERIFY(bf->readyToGo());
    QVERIFY(16 == bf->encrypt((void *)data, 16));

    delete bf;
    bf = new BlowFish();
    bf->setKey((void *)key, 7 * 8);

    QVERIFY(16 == bf->decrypt((void *)data, 16));
    QVERIFY(strcmp(data, "This is a test.") == 0);
    delete bf;
}

QTEST_GUILESS_MAIN(KWalletCBCTest)

#include "moc_kwalletcbc.cpp"
