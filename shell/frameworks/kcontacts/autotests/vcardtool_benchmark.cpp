/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcardtool_benchmark.h"
#include "title.h"
#include "vcardtool_p.h"
#include <QTest>

PerformanceTest::PerformanceTest(QObject *parent)
    : QObject(parent)
{
}

PerformanceTest::~PerformanceTest()
{
}

void PerformanceTest::testParserPerformance()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Title:boo\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    QBENCHMARK {
        KContacts::VCardTool vcard;
        const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
        QCOMPARE(lst.count(), 1);
    }
}

void PerformanceTest::testExportPerformance()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Title::List lstTitle;
    KContacts::Title title(QStringLiteral("fr"));
    lstTitle << title;
    addr.setExtraTitleList(lstTitle);
    lst << addr;
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TITLE:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QBENCHMARK {
        KContacts::VCardTool vcard;
        const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
        QCOMPARE(ba, expected);
    }
}

QTEST_GUILESS_MAIN(PerformanceTest)

#include "moc_vcardtool_benchmark.cpp"
