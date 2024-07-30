/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mobileproviders.h"
#include <QTest>

class MobileProvidersTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testProviders();
    void testProviderFromMccMnc();
    void testProviderFromMccMnc_data();
};

void MobileProvidersTest::testProviders()
{
    MobileProviders providers;

    const QStringList germanProviders = providers.getProvidersList(QStringLiteral("DE"), NetworkManager::ConnectionSettings::Gsm);

    QVERIFY(germanProviders.contains(QStringLiteral("Vodafone")));

    const QStringList vodafoneApns = providers.getApns(QStringLiteral("Vodafone"));
    QVERIFY(!vodafoneApns.isEmpty());

    const QVariantMap apnInfo = providers.getApnInfo(vodafoneApns.constFirst());
    QVERIFY(apnInfo.contains(QStringLiteral("apn")));
}

void MobileProvidersTest::testProviderFromMccMnc()
{
    QFETCH(QString, mccmnc);
    QFETCH(QStringList, providerNames);

    MobileProviders providers;

    QCOMPARE(providers.getProvidersFromMCCMNC(mccmnc), providerNames);
}

void MobileProvidersTest::testProviderFromMccMnc_data()
{
    QTest::addColumn<QString>("mccmnc");
    QTest::addColumn<QStringList>("providerNames");

    QTest::newRow("Aldi 1") << "26277" << QStringList{"AldiTalk/MedionMobile", "blau.de", "E-Plus", "simyo Internet"};
    QTest::newRow("Aldi 2") << "26203" << QStringList{"AldiTalk/MedionMobile", "blau.de", "E-Plus", "simyo Internet"};
    QTest::newRow("Vodafone CZ") << "23003" << QStringList{"Vodafone"};
    QTest::newRow("1&1") << "26202" << QStringList{"Bild Mobil", "Vodafone", "1&1"};
}

QTEST_GUILESS_MAIN(MobileProvidersTest)

#include "mobileproviderstest.moc"
