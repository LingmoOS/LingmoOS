/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "secrecytest.h"
#include "kcontacts/secrecy.h"
#include "vcardtool_p.h"
#include <QTest>

QTEST_MAIN(SecrecyTest)

void SecrecyTest::emptyTest()
{
    KContacts::Secrecy secrecy;

    QVERIFY(secrecy.type() == KContacts::Secrecy::Invalid);
}

void SecrecyTest::storeTest()
{
    KContacts::Secrecy secrecy;

    secrecy.setType(KContacts::Secrecy::Private);

    QVERIFY(secrecy.type() == KContacts::Secrecy::Private);
}

void SecrecyTest::equalsTest()
{
    KContacts::Secrecy secrecy1;
    KContacts::Secrecy secrecy2;

    secrecy1.setType(KContacts::Secrecy::Confidential);
    secrecy2.setType(KContacts::Secrecy::Confidential);

    QVERIFY(secrecy1 == secrecy2);
}

void SecrecyTest::differsTest()
{
    KContacts::Secrecy secrecy1(KContacts::Secrecy::Private);
    KContacts::Secrecy secrecy2(KContacts::Secrecy::Confidential);

    QVERIFY(secrecy1 != secrecy2);
}

void SecrecyTest::assignmentTest()
{
    KContacts::Secrecy secrecy1;
    KContacts::Secrecy secrecy2;

    secrecy1.setType(KContacts::Secrecy::Confidential);
    secrecy1 = secrecy2;

    QVERIFY(secrecy1 == secrecy2);
}

void SecrecyTest::serializeTest()
{
    KContacts::Secrecy secrecy1;
    KContacts::Secrecy secrecy2;

    secrecy1.setType(KContacts::Secrecy::Confidential);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << secrecy1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> secrecy2;

    QVERIFY(secrecy1 == secrecy2);
}

#include "moc_secrecytest.cpp"
