/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcardlinetest.h"
#include "vcardline_p.h"
#include <QTest>

VCardLineTest::VCardLineTest(QObject *parent)
    : QObject(parent)
{
}

VCardLineTest::~VCardLineTest()
{
}

void VCardLineTest::shouldHaveDefaultValue()
{
    KContacts::VCardLine line;
    QVERIFY(line.identifier().isEmpty());
    QVERIFY(line.group().isEmpty());
    QVERIFY(line.parameterList().isEmpty());
    QVERIFY(line.value().isNull());
}

void VCardLineTest::shouldAssignValues()
{
    KContacts::VCardLine line;
    const QString identifier(QStringLiteral("foo"));
    const QString group(QStringLiteral("bla"));
    line.setIdentifier(identifier);
    line.setGroup(group);
    QMap<QString, QString> map;
    for (int i = 0; i < 5; ++i) {
        map.insert(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
        line.addParameter(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
    }
    const QVariant valueVariant = QVariant(QStringLiteral("a"));
    line.setValue(valueVariant);

    QVERIFY(!line.parameterList().isEmpty());
    QCOMPARE(line.parameterList(), QStringList() << map.keys());

    QCOMPARE(line.identifier(), identifier);

    QCOMPARE(line.group(), group);
    QCOMPARE(line.value(), valueVariant);
}

void VCardLineTest::shouldCopyValue()
{
    KContacts::VCardLine line;
    const QString identifier(QStringLiteral("foo"));
    const QString group(QStringLiteral("bla"));
    line.setIdentifier(identifier);
    line.setGroup(group);
    QMap<QString, QString> map;
    for (int i = 0; i < 5; ++i) {
        map.insert(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
        line.addParameter(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
    }
    const QVariant valueVariant = QVariant(QStringLiteral("a"));
    line.setValue(valueVariant);

    QVERIFY(!line.parameterList().isEmpty());
    QCOMPARE(line.parameterList(), QStringList() << map.keys());

    QCOMPARE(line.identifier(), identifier);

    QCOMPARE(line.group(), group);
    QCOMPARE(line.value(), valueVariant);

    KContacts::VCardLine copyLine(line);
    QVERIFY(!copyLine.parameterList().isEmpty());
    QCOMPARE(copyLine.parameterList(), QStringList() << map.keys());

    QCOMPARE(copyLine.identifier(), identifier);

    QCOMPARE(copyLine.group(), group);
    QCOMPARE(copyLine.value(), valueVariant);
}

void VCardLineTest::shouldEqualValue()
{
    KContacts::VCardLine line;
    const QString identifier(QStringLiteral("foo"));
    const QString group(QStringLiteral("bla"));
    line.setIdentifier(identifier);
    line.setGroup(group);
    QMap<QString, QString> map;
    for (int i = 0; i < 5; ++i) {
        map.insert(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
        line.addParameter(QStringLiteral("foo%1").arg(i), QStringLiteral("bla%1").arg(i));
    }
    const QVariant valueVariant = QVariant(QStringLiteral("a"));
    line.setValue(valueVariant);

    QVERIFY(!line.parameterList().isEmpty());
    QCOMPARE(line.parameterList(), QStringList() << map.keys());

    QCOMPARE(line.identifier(), identifier);

    QCOMPARE(line.group(), group);
    QCOMPARE(line.value(), valueVariant);

    KContacts::VCardLine copyLine(line);
    QVERIFY(line == copyLine);
}

QTEST_MAIN(VCardLineTest)

#include "moc_vcardlinetest.cpp"
