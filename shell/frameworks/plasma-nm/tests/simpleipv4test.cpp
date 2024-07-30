/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "simpleipv4addressvalidator.h"
#include <QTest>

class SimpleIpv4Test : public QObject
{
    Q_OBJECT

public:
    SimpleIpv4Test();

private Q_SLOTS:
    void baseTest();
    void baseTest_data();
    void cidrTest();
    void cidrTest_data();
    void portTest();
    void portTest_data();

private:
    SimpleIpV4AddressValidator m_vb;
    SimpleIpV4AddressValidator m_vc;
    SimpleIpV4AddressValidator m_vp;
};

SimpleIpv4Test::SimpleIpv4Test()
    : m_vb(SimpleIpV4AddressValidator::AddressStyle::Base)
    , m_vc(SimpleIpV4AddressValidator::AddressStyle::WithCidr)
    , m_vp(SimpleIpV4AddressValidator::AddressStyle::WithPort)
{
}

Q_DECLARE_METATYPE(QValidator::State)

void SimpleIpv4Test::baseTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("empty string") << "" << QValidator::Intermediate;
    QTest::newRow("123.12.2") << "123.12.2" << QValidator::Intermediate;
    QTest::newRow("123.45.22.9") << "123.45.22.9" << QValidator::Acceptable;
    QTest::newRow("1") << "1" << QValidator::Intermediate;
    QTest::newRow("12") << "12" << QValidator::Intermediate;
    QTest::newRow("123") << "123" << QValidator::Intermediate;
    QTest::newRow("123.") << "123." << QValidator::Intermediate;
    QTest::newRow("123.4") << "123.4" << QValidator::Intermediate;
    QTest::newRow("123.54") << "123.54" << QValidator::Intermediate;
    QTest::newRow("123.54.") << "123.54." << QValidator::Intermediate;
    QTest::newRow("123.255") << "123.255" << QValidator::Intermediate;
    QTest::newRow("123.255.") << "123.255." << QValidator::Intermediate;
    QTest::newRow("123.123.9") << "123.123.9" << QValidator::Intermediate;
    QTest::newRow("123.34.99") << "123.34.99" << QValidator::Intermediate;
    QTest::newRow("123.22.233") << "123.22.233" << QValidator::Intermediate;
    QTest::newRow("255.255.255.") << "255.255.255." << QValidator::Intermediate;
    QTest::newRow("1.1.1.7") << "1.1.1.7" << QValidator::Acceptable;
    QTest::newRow("12a") << "12a" << QValidator::Invalid;
    QTest::newRow("255") << "255" << QValidator::Intermediate;
    QTest::newRow("256") << "256" << QValidator::Invalid;
    QTest::newRow("255.255.255.255") << "255.255.255.255" << QValidator::Acceptable;
    QTest::newRow("256.255.255.255") << "256.255.255.255" << QValidator::Invalid;
    QTest::newRow("255.256.255.255") << "255.256.255.255" << QValidator::Invalid;
    QTest::newRow("255.255.256.255") << "255.255.256.255" << QValidator::Invalid;
    QTest::newRow("255.255.255.256") << "255.255.255.256" << QValidator::Invalid;
    QTest::newRow("1.1.1.1.") << "1.1.1.1." << QValidator::Invalid;
}

void SimpleIpv4Test::baseTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vb.validate(address, pos), result);
}

void SimpleIpv4Test::cidrTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("10.77.18.4/32") << "10.77.18.4/32" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4/33") << "10.77.18.4/33" << QValidator::Invalid;
    QTest::newRow("10.77.14.8") << "10.77.14.8" << QValidator::Intermediate;
    QTest::newRow("10.77.13/") << "10.77.13/" << QValidator::Invalid;
    QTest::newRow("33.22.55.44/") << "33.22.55.44/" << QValidator::Intermediate;
    QTest::newRow("11.23.45./") << "11.23.45./" << QValidator::Invalid;
    QTest::newRow("0.0.0.0/0") << "0.0.0.0/0" << QValidator::Acceptable;
    QTest::newRow("1.2.3.4/28/") << "1.2.3.4/28/" << QValidator::Invalid;
    QTest::newRow("1.2.3.4//") << "1.2.3.4//" << QValidator::Invalid;
}

void SimpleIpv4Test::cidrTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vc.validate(address, pos), result);
}

void SimpleIpv4Test::portTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("10.77.18.4:32") << "10.77.18.4:32" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4:65536") << "10.77.18.4:65536" << QValidator::Invalid;
    QTest::newRow("10.77.18.4") << "10.77.18.4" << QValidator::Intermediate;
    QTest::newRow("10.77.13:") << "10.77.13:" << QValidator::Invalid;
    QTest::newRow("33.22.55.44:") << "33.22.55.44:" << QValidator::Intermediate;
    QTest::newRow("11.23.45.:") << "11.23.45.:" << QValidator::Invalid;
    QTest::newRow("0.0.0.0:65535") << "0.0.0.0:65535" << QValidator::Acceptable;
    QTest::newRow("1.2.3.4:0:") << "1.2.3.4:0:" << QValidator::Invalid;
    QTest::newRow("1.2.3.4::") << "1.2.3.4::" << QValidator::Invalid;
}

void SimpleIpv4Test::portTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vp.validate(address, pos), result);
}

QTEST_APPLESS_MAIN(SimpleIpv4Test)

#include "simpleipv4test.moc"
