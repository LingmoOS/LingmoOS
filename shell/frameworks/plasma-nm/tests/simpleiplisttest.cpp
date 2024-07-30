/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "simpleiplistvalidator.h"
#include <QTest>

class SimpleipListTest : public QObject
{
    Q_OBJECT

public:
    SimpleipListTest();

private Q_SLOTS:
    void baseTest();
    void baseTest_data();
    void cidrTest();
    void cidrTest_data();
    void portTest();
    void portTest_data();

private:
    SimpleIpListValidator m_vb;
    SimpleIpListValidator m_vc;
    SimpleIpListValidator m_vp;
};

SimpleipListTest::SimpleipListTest()
    : m_vb(SimpleIpListValidator::AddressStyle::Base)
    , m_vc(SimpleIpListValidator::AddressStyle::WithCidr)
    , m_vp(SimpleIpListValidator::AddressStyle::WithPort)
{
}

Q_DECLARE_METATYPE(QValidator::State)

void SimpleipListTest::baseTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("") << "" << QValidator::Intermediate;
    QTest::newRow("123.12.2") << "123.12.2" << QValidator::Intermediate;
    QTest::newRow("123.12.2,") << "123.12.2," << QValidator::Invalid;
    QTest::newRow("123.45.22.9") << "123.45.22.9" << QValidator::Acceptable;
    QTest::newRow("123.45.22.9,") << "123.45.22.9," << QValidator::Intermediate;
    QTest::newRow("123.45.22.9,  ") << "123.45.22.9,  " << QValidator::Intermediate;
    QTest::newRow("123.45.22,") << "123.45.22," << QValidator::Invalid;
    QTest::newRow("123.45.22.9,  BBEf:0112") << "123.45.22.9,  BBEf:0112" << QValidator::Intermediate;
    QTest::newRow("123.45.22.9,  BBEf:0112::1") << "123.45.22.9,  BBEf:0112::1" << QValidator::Acceptable;
    QTest::newRow("123.45.22.9,  BBEf:0112:,") << "123.45.22.9,  BBEf:0112:," << QValidator::Invalid;
    QTest::newRow("123.45.22.9,  BBEf:0112::1,") << "123.45.22.9,  BBEf:0112::1," << QValidator::Intermediate;
    QTest::newRow("123.45.22.9,  BBEf:0112::1/123,") << "123.45.22.9,  BBEf:0112::1/123," << QValidator::Invalid;
    QTest::newRow("123.45.22.9,  BBEf:0112::1,1.2.3.4") << "123.45.22.9,  BBEf:0112::1,1.2.3.4" << QValidator::Acceptable;
}

void SimpleipListTest::baseTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vb.validate(address, pos), result);
}

void SimpleipListTest::cidrTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("10.77.18.4/32") << "10.77.18.4/32" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4/32,") << "10.77.18.4/32," << QValidator::Intermediate;
    QTest::newRow("10.77.18.4/32,Be00:e00:0:") << "10.77.18.4/32,Be00:e00:0:" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4/32,Be00:e00:0:21/3") << "10.77.18.4/32,Be00:e00:0:21/3" << QValidator::Invalid;
    QTest::newRow("10.77.18.4/32,Be00:e00::0:21/128") << "10.77.18.4/32,Be00:e00::0:21/128" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4/32,Be00:e00::0:21/129") << "10.77.18.4/32,Be00:e00::0:21/129" << QValidator::Invalid;
}

void SimpleipListTest::cidrTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vc.validate(address, pos), result);
}

void SimpleipListTest::portTest_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("10.77.18.4:32") << "10.77.18.4:32" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4:,") << "10.77.18.4:," << QValidator::Invalid;
    QTest::newRow("10.77.18.4:234, 1") << "10.77.18.4:234, 1" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4:234, 1b") << "10.77.18.4:234, 1b" << QValidator::Invalid;
    QTest::newRow("10.77.18.4:234, [1b") << "10.77.18.4:234, [1b" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4:234, [10:33:22::1") << "10.77.18.4:234, [10:33:22::1" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4:234, [10:33:22::1]") << "10.77.18.4:234, [10:33:22::1]" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4:234, [10:33:22::1]:") << "10.77.18.4:234, [10:33:22::1]:" << QValidator::Intermediate;
    QTest::newRow("10.77.18.4:234, [10:33:22::1]:22") << "10.77.18.4:234, [10:33:22::1]:22" << QValidator::Acceptable;
    QTest::newRow("10.77.18.4:234, [10:33:22::1]:22,12.23.34.45:65535") << "10.77.18.4:234, [10:33:22::1]:22,12.23.34.45:65535" << QValidator::Acceptable;
}

void SimpleipListTest::portTest()
{
    int pos;

    QFETCH(QString, address);
    QFETCH(QValidator::State, result);

    QCOMPARE(m_vp.validate(address, pos), result);
}

QTEST_GUILESS_MAIN(SimpleipListTest)

#include "simpleiplisttest.moc"
