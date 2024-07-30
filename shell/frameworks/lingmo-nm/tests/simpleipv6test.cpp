/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "simpleipv6addressvalidator.h"
#include <QTest>

class SimpleIpv6Test : public QObject
{
    Q_OBJECT

public:
    SimpleIpv6Test();

private Q_SLOTS:
    void baseTest();
    void baseTest_data();
    void cidrTest();
    void cidrTest_data();
    void portTest();
    void portTest_data();

private:
    SimpleIpV6AddressValidator m_vb;
    SimpleIpV6AddressValidator m_vc;
    SimpleIpV6AddressValidator m_vp;
};

SimpleIpv6Test::SimpleIpv6Test()
    : m_vb(SimpleIpV6AddressValidator::AddressStyle::Base)
    , m_vc(SimpleIpV6AddressValidator::AddressStyle::WithCidr)
    , m_vp(SimpleIpV6AddressValidator::AddressStyle::WithPort)
{
}

Q_DECLARE_METATYPE(QValidator::State)

void SimpleIpv6Test::baseTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("null string") << "" << QValidator::Intermediate;
    QTest::newRow("0123:4567:89ab:cdef") << "0123:4567:89ab:cdef" << QValidator::Intermediate;
    QTest::newRow("0123:4567:89ab:cdef:0123:4567:89ab:cdef") << "0123:4567:89ab:cdef:0123:4567:89ab:cdef" << QValidator::Acceptable;
    QTest::newRow("1") << "1" << QValidator::Intermediate;
    QTest::newRow("12") << "12" << QValidator::Intermediate;
    QTest::newRow("123") << "123" << QValidator::Intermediate;
    QTest::newRow("1234") << "1234" << QValidator::Intermediate;
    QTest::newRow("1234:") << "1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1") << "1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:12") << "1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:123") << "1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234") << "1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:") << "1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1") << "1234:1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:1234:12") << "1234:1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:1234:123") << "1234:1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234") << "1234:1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:") << "1234:1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1") << "1234:1234:1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:12") << "1234:1234:1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:123") << "1234:1234:1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234") << "1234:1234:1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:") << "1234:1234:1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1") << "1234:1234:1234:1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:12") << "1234:1234:1234:1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:123") << "1234:1234:1234:1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234") << "1234:1234:1234:1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:") << "1234:1234:1234:1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1") << "1234:1234:1234:1234:1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:12") << "1234:1234:1234:1234:1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:123") << "1234:1234:1234:1234:1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234") << "1234:1234:1234:1234:1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:") << "1234:1234:1234:1234:1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1") << "1234:1234:1234:1234:1234:1234:1" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:12") << "1234:1234:1234:1234:1234:1234:12" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:123") << "1234:1234:1234:1234:1234:1234:123" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234") << "1234:1234:1234:1234:1234:1234:1234" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234:") << "1234:1234:1234:1234:1234:1234:1234:" << QValidator::Intermediate;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234:1") << "1234:1234:1234:1234:1234:1234:1234:1" << QValidator::Acceptable;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234:12") << "1234:1234:1234:1234:1234:1234:1234:12" << QValidator::Acceptable;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234:123") << "1234:1234:1234:1234:1234:1234:1234:123" << QValidator::Acceptable;
    QTest::newRow("1234:1234:1234:1234:1234:1234:1234:1234") << "1234:1234:1234:1234:1234:1234:1234:1234" << QValidator::Acceptable;
    QTest::newRow(":") << ":" << QValidator::Intermediate;
    QTest::newRow("::") << "::" << QValidator::Acceptable;
    QTest::newRow("::1:2:3:4") << "::1:2:3:4" << QValidator::Acceptable;
    QTest::newRow("1::2:3:4") << "1::2:3:4" << QValidator::Acceptable;
    QTest::newRow("1:2345::6:7") << "1:2345::6:7" << QValidator::Acceptable;
    QTest::newRow("1:2:3::1:1:1") << "1:2:3::1:1:1" << QValidator::Acceptable;
    QTest::newRow("1:2:3:4::1") << "1:2:3:4::1" << QValidator::Acceptable;
    QTest::newRow("1:2:3:4:5::1") << "1:2:3:4:5::1" << QValidator::Acceptable;
    QTest::newRow("1:2:3:4:5:6::1") << "1:2:3:4:5:6::1" << QValidator::Acceptable;
    QTest::newRow("1:2:3:4:5:6:7::") << "1:2:3:4:5:6:7::" << QValidator::Acceptable;
    QTest::newRow("1::") << "1::" << QValidator::Acceptable;
    QTest::newRow("::1:2:3:4") << "::1:2:3:4" << QValidator::Acceptable;
    QTest::newRow("::1:2:3::4") << "::1:2:3::4" << QValidator::Invalid;
    QTest::newRow("1:2::3:4:5:6:7:") << "1:2::3:4:5:6:7:" << QValidator::Invalid;
    QTest::newRow("1:2::3:4:5:6:7:8") << "1:2::3:4:5:6:7:8" << QValidator::Invalid;
    QTest::newRow(":1") << ":1" << QValidator::Invalid;
    QTest::newRow("0123:4567:89ab:cdef:0123:4567:89ab:cdeg") << "0123:4567:89ab:cdef:0123:4567:89ab:cdeg" << QValidator::Invalid;
    QTest::newRow("0123:4567:89ab:cdef:0123:4567:89ab:cde.") << "0123:4567:89ab:cdef:0123:4567:89ab:cde." << QValidator::Invalid;
    QTest::newRow("0123:4567:89ab:cdef:0123:4567:89ab:cden") << "0123:4567:89ab:cdef:0123:4567:89ab:cden" << QValidator::Invalid;
    QTest::newRow("0n") << "0n" << QValidator::Invalid;
}

void SimpleIpv6Test::baseTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vb.validate(address, pos), result);
}

void SimpleIpv6Test::cidrTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("1234:2345:3456:4567:5678:6789:789A:89ab/128") << "1234:2345:3456:4567:5678:6789:789A:89ab/128" << QValidator::Acceptable;
    QTest::newRow("") << "" << QValidator::Intermediate;
    QTest::newRow("1234:2345::6789:789A:89ab/28") << "1234:2345::6789:789A:89ab/28" << QValidator::Acceptable;
    QTest::newRow("1234:2345:3456:4567:5678:6789:789A:89ab/129") << "1234:2345:3456:4567:5678:6789:789A:89ab/129" << QValidator::Invalid;
    QTest::newRow("1234:2345:3456:4567:5678:6789:789A:89ab/") << "1234:2345:3456:4567:5678:6789:789A:89ab/" << QValidator::Intermediate;
    QTest::newRow("1234:2345:3456:4567:5678:6789:789A/") << "1234:2345:3456:4567:5678:6789:789A/" << QValidator::Invalid;
    QTest::newRow("1234:2345:/") << "1234:2345:/" << QValidator::Invalid;
    QTest::newRow("1234:2345::6789:789A:89ab") << "1234:2345::6789:789A:89ab" << QValidator::Intermediate;
    QTest::newRow("::/0") << "::/0" << QValidator::Acceptable;
    QTest::newRow("1234:2345::6789:789A:89ab/28/") << "1234:2345::6789:789A:89ab/28/" << QValidator::Invalid;
    QTest::newRow("1234:2345::6789:789A:89ab//") << "1234:2345::6789:789A:89ab//" << QValidator::Invalid;
}

void SimpleIpv6Test::cidrTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vc.validate(address, pos), result);
}

void SimpleIpv6Test::portTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("") << "" << QValidator::Intermediate;
    QTest::newRow("1") << "1" << QValidator::Invalid;
    QTest::newRow("[1") << "[1" << QValidator::Intermediate;
    QTest::newRow("[1123:22:44:11]") << "[1123:22:44:11]" << QValidator::Invalid;
    QTest::newRow("[1234:2345::6789:789A:89ab]") << "[1234:2345::6789:789A:89ab]" << QValidator::Intermediate;
    QTest::newRow("[1234:2345::6789:789A:89ab]:") << "[1234:2345::6789:789A:89ab]:" << QValidator::Intermediate;
    QTest::newRow("[1234:2345::6789:789A:89ab]:a") << "[1234:2345::6789:789A:89ab]:a" << QValidator::Invalid;
    QTest::newRow("[1234:2345::6789:789A:89ab]:12") << "[1234:2345::6789:789A:89ab]:12" << QValidator::Acceptable;
    QTest::newRow("[1234:2345::6789:789A:89ab]:65535") << "[1234:2345::6789:789A:89ab]:65535" << QValidator::Acceptable;
    QTest::newRow("[1234:2345::6789:789A:89ab]:65536") << "[1234:2345::6789:789A:89ab]:65536" << QValidator::Invalid;
}

void SimpleIpv6Test::portTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vp.validate(address, pos), result);
}

QTEST_GUILESS_MAIN(SimpleIpv6Test)

#include "simpleipv6test.moc"
