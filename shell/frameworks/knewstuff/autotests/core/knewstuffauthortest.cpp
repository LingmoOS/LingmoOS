/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// unit test for author

#include <QString>
#include <QTest>

#include "core/author.h"

const QString name = QStringLiteral("Name");
const QString email = QStringLiteral("Email@nowhere.com");
const QString jabber = QStringLiteral("something@kdetalk.net");
const QString homepage = QStringLiteral("http://www.myhomepage.com");

class testAuthor : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProperties();
    void testCopy();
    void testAssignment();
};

void testAuthor::testProperties()
{
    KNSCore::Author author;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    QCOMPARE(author.name(), name);
    QCOMPARE(author.email(), email);
    QCOMPARE(author.jabber(), jabber);
    QCOMPARE(author.homepage(), homepage);
}

void testAuthor::testCopy()
{
    KNSCore::Author author;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    KNSCore::Author author2(author);
    QCOMPARE(author.name(), author2.name());
    QCOMPARE(author.email(), author2.email());
    QCOMPARE(author.jabber(), author2.jabber());
    QCOMPARE(author.homepage(), author2.homepage());
}

void testAuthor::testAssignment()
{
    KNSCore::Author author;
    KNSCore::Author author2;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    author2 = author;
    QCOMPARE(author.name(), author2.name());
    QCOMPARE(author.email(), author2.email());
    QCOMPARE(author.jabber(), author2.jabber());
    QCOMPARE(author.homepage(), author2.homepage());
}

QTEST_GUILESS_MAIN(testAuthor)
#include "knewstuffauthortest.moc"
