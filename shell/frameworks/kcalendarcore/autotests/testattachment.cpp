/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testattachment.h"
#include "attachment.h"
#include "event.h"

#include <QTest>
QTEST_MAIN(AttachmentTest)

using namespace KCalendarCore;

void AttachmentTest::testValidity()
{
    Attachment attachment(QStringLiteral("http://www.kde.org"));
    QCOMPARE(attachment.uri(), QStringLiteral("http://www.kde.org"));
    QCOMPARE(attachment.data(), QByteArray());
    QVERIFY(attachment.decodedData().isEmpty());
    QVERIFY(!attachment.isBinary());

    attachment.setDecodedData("foo");
    QVERIFY(attachment.isBinary());
    QCOMPARE(attachment.decodedData(), QByteArray("foo"));
    QCOMPARE(attachment.data(), QByteArray("Zm9v"));
    QCOMPARE(attachment.size(), 3U);

    Attachment attachment2 = Attachment(QByteArray("Zm9v"));
    QCOMPARE(attachment2.size(), 3U);
    QCOMPARE(attachment2.decodedData(), QByteArray("foo"));
    attachment2.setDecodedData("123456");
    QCOMPARE(attachment2.size(), 6U);

    Attachment attachment3(attachment2);
    QCOMPARE(attachment3.size(), attachment2.size());

    QByteArray fred("jkajskldfasjfklasjfaskfaskfasfkasfjdasfkasjf");
    Attachment attachment4(fred, QStringLiteral("image/nonsense"));
    QCOMPARE(fred, attachment4.data());
    QVERIFY(attachment4.isBinary());
    QByteArray ethel("a9fafafjafkasmfasfasffksjklfjau");
    attachment4.setData(ethel);
    QCOMPARE(ethel, attachment4.data());

    Attachment attachment5(QStringLiteral("http://www.kde.org"));
    Attachment attachment6(QStringLiteral("http://www.kde.org"));
    QVERIFY(attachment5 == attachment6);
    attachment5.setUri(QStringLiteral("http://bugs.kde.org"));
    QVERIFY(attachment5 != attachment6);
    attachment5.setDecodedData("123456");
    attachment6.setDecodedData("123456");
    QVERIFY(attachment5 == attachment6);
    attachment6.setDecodedData("12345");
    QVERIFY(attachment5 != attachment6);
}

void AttachmentTest::testSerializer_data()
{
    QTest::addColumn<KCalendarCore::Attachment>("attachment");

    Attachment nonInline(QStringLiteral("http://www.kde.org"));
    Attachment inlineAttachment(QByteArray("foo"), QStringLiteral("image/nonsense"));

    QTest::newRow("inline") << inlineAttachment;
    QTest::newRow("not inline") << nonInline;
}

void AttachmentTest::testSerializer()
{
    QFETCH(KCalendarCore::Attachment, attachment);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << attachment; // Serialize

    Attachment attachment2(QStringLiteral("foo"));
    QVERIFY(attachment != attachment2);
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> attachment2; // deserialize
    QVERIFY(attachment == attachment2);
}

#include "moc_testattachment.cpp"
