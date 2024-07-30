// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Daniel Vrátil <dvratil@kde.org>

#include "ipvalidator.h"

#include <QTest>

class IPValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIPv4Validation_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QValidator::State>("state");

        QTest::newRow("1") << QStringLiteral("1") << QValidator::Intermediate;
        QTest::newRow("10.") << QStringLiteral("10.") << QValidator::Intermediate;
        QTest::newRow("10.a") << QStringLiteral("10.a") << QValidator::Invalid;
        QTest::newRow("10.0.0") << QStringLiteral("10.0.0") << QValidator::Intermediate;
        QTest::newRow("10.0.0.0") << QStringLiteral("10.0.0.0") << QValidator::Acceptable;
        QTest::newRow("10.0.0.0/") << QStringLiteral("10.0.0.0/") << QValidator::Intermediate;
        QTest::newRow("10.0.0.0/1") << QStringLiteral("10.0.0.0/1") << QValidator::Acceptable;
        QTest::newRow("10.0.0.0/10") << QStringLiteral("10.0.0.0/10") << QValidator::Acceptable;
        QTest::newRow("10.0.0.0/100") << QStringLiteral("10.0.0.0/100") << QValidator::Invalid;
        QTest::newRow("192.168.0.256") << QStringLiteral("192.168.0.256") << QValidator::Intermediate;
        QTest::newRow("192.168.0.0/.") << QStringLiteral("192.168.0.0/.") << QValidator::Invalid;
        QTest::newRow("::1") << QStringLiteral("::1") << QValidator::Invalid;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850/64") << QStringLiteral("fe80::39fe:37cd:6d9d:9850/64") << QValidator::Invalid;
    }

    void testIPv4Validation()
    {
        QFETCH(QString, input);
        QFETCH(QValidator::State, state);

        IPValidator validator;
        QCOMPARE(validator.ipVersion(), IPValidator::IPVersion::IPv4); // default
        int pos = 0;
        QCOMPARE(validator.validate(input, pos), state);
    }

    void testIPv6Validation_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QValidator::State>("state");

        QTest::newRow("::1") << QStringLiteral("::1") << QValidator::Acceptable;
        QTest::newRow("fe80:::") << QStringLiteral("fe80:::") << QValidator::Intermediate;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850") << QStringLiteral("fe80::39fe:37cd:6d9d:9850") << QValidator::Acceptable;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850/") << QStringLiteral("fe80::39fe:37cd:6d9d:9850/") << QValidator::Intermediate;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850/64") << QStringLiteral("fe80::39fe:37cd:6d9d:9850/64") << QValidator::Acceptable;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850/fe") << QStringLiteral("fe80::39fe:37cd:6d9d:9850/fe") << QValidator::Invalid;
        QTest::newRow("fe80::39fe:37cd:6d9d:9850/190") << QStringLiteral("fe80::39fe:37cd:6d9d:9850/190") << QValidator::Invalid;
        QTest::newRow("fg80::39fe") << QStringLiteral("fg80::39fe") << QValidator::Invalid;
        QTest::newRow("192.168.0.0") << QStringLiteral("192.168.0.1") << QValidator::Intermediate;
        QTest::newRow("192.168.0.0/24") << QStringLiteral("192.168.0.0/24") << QValidator::Intermediate;
        QTest::newRow("::FFFF:1.2.3.4") << QStringLiteral("::FFFF:1.2.3.4") << QValidator::Acceptable;
        QTest::newRow("::FFFF:1.2.0.0/112") << QStringLiteral("::FFFF:1.2.0.0/112") << QValidator::Acceptable;
    }

    void testIPv6Validation()
    {
        QFETCH(QString, input);
        QFETCH(QValidator::State, state);

        IPValidator validator;
        validator.setIPVersion(IPValidator::IPVersion::IPv6);
        int pos = 0;
        QCOMPARE(validator.validate(input, pos), state);
    }
};

QTEST_MAIN(IPValidatorTest)

#include "ipvalidatortest.moc"