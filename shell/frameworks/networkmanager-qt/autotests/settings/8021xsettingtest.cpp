/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "8021xsettingtest.h"

#include "settings/security8021xsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void Security8021xSettingTest::testSetting_data()
{
    QTest::addColumn<QStringList>("eap");
    QTest::addColumn<QString>("identity");
    QTest::addColumn<QString>("anonymousIdentity");
    QTest::addColumn<QString>("pacFile");
    QTest::addColumn<QByteArray>("caCert");
    QTest::addColumn<QString>("domainSuffixMatch");
    QTest::addColumn<QString>("caPath");
    QTest::addColumn<QString>("subjectMatch");
    QTest::addColumn<QStringList>("altSubjectMatches");
    QTest::addColumn<QByteArray>("clientCert");
    QTest::addColumn<QString>("phase1PeapVer");
    QTest::addColumn<QString>("phase1PeapLabel");
    QTest::addColumn<QString>("phase1FastProvisioning");
    QTest::addColumn<QString>("phase2Auth");
    QTest::addColumn<QString>("phase2EapAuth");
    QTest::addColumn<QByteArray>("phase2CaCert");
    QTest::addColumn<QString>("phase2CaPath");
    QTest::addColumn<QString>("phase2SubjectMatch");
    QTest::addColumn<QStringList>("phase2AltSubjectMatches");
    QTest::addColumn<QByteArray>("phase2ClientCert");
    QTest::addColumn<QString>("password");
    QTest::addColumn<quint32>("passwordFlags");
    QTest::addColumn<QByteArray>("passwordRaw");
    QTest::addColumn<quint32>("passwordRawFlags");
    QTest::addColumn<QByteArray>("privateKey");
    QTest::addColumn<QString>("privateKeyPassword");
    QTest::addColumn<quint32>("privateKeyPasswordFlags");
    QTest::addColumn<QByteArray>("phase2PrivateKey");
    QTest::addColumn<QString>("phase2PrivateKeyPassword");
    QTest::addColumn<quint32>("phase2PrivateKeyPasswordFlags");
    QTest::addColumn<QString>("pin");
    QTest::addColumn<quint32>("pinFlags");
    QTest::addColumn<bool>("systemCaCerts");

    QStringList eapMethods;
    eapMethods << "leap"
               << "md5"
               << "tls"
               << "peap"
               << "ttls"
               << "fast";

    QStringList altSubjectMatches;
    altSubjectMatches << "subject1"
                      << "subject2";

    QTest::newRow("setting1") << eapMethods // eap
                              << QString("identity") // identity
                              << QString("anonymousIdentity") // anonymousIdentity
                              << QString("file://path/to/pac/file") // pacFile
                              << QByteArray("file://path/to/ca/cert") // caCert
                              << QString("example.com") // domainSuffixMatch
                              << QString("file://path/to/ca/cert") // caPath
                              << QString("subjectMatch") // subjectMatch
                              << altSubjectMatches // altSubjectMatches
                              << QByteArray("file://path/to/client/cert") // clientCert
                              << QString("1") // phase1PeapVer
                              << QString("1") // phase1PeapLabel
                              << QString("3") // phase1FastProvisioning
                              << QString("mschapv2") // phase2Auth
                              << QString("tls") // phase2EapAuth
                              << QByteArray("file://path/to/phase2/ca/cert") // phase2CaCert
                              << QString("file://path/to/phase2/ca/cert") // phase2CaPath
                              << QString("phase2SubjectMatch") // phase2SubjectMatch
                              << altSubjectMatches // phase2AltSubjectMatches
                              << QByteArray("file://path/to/phase2/client/cert") // phase2ClientCert
                              << QString("password") // password
                              << (quint32)2 // passwordFlags
                              << QByteArray("passwordRaw") // passwordRaw
                              << (quint32)4 // passwordFlags
                              << QByteArray("privateKey") // privateKey
                              << QString("privateKeyPassword") // privateKeyPassword
                              << (quint32)2 // privateKeyPasswordFlags
                              << QByteArray("phase2PrivateKey") // phase2PrivateKey
                              << QString("phase2PrivateKeyPassword") // phase2PrivateKeyPassword
                              << (quint32)4 // phase2PrivateKeyPasswordFlags
                              << QString("pin") // pin
                              << (quint32)2 // pinFlags
                              << true; // systemCaCerts
}

void Security8021xSettingTest::testSetting()
{
    QFETCH(QStringList, eap);
    QFETCH(QString, identity);
    QFETCH(QString, anonymousIdentity);
    QFETCH(QString, pacFile);
    QFETCH(QByteArray, caCert);
    QFETCH(QString, domainSuffixMatch);
    QFETCH(QString, caPath);
    QFETCH(QString, subjectMatch);
    QFETCH(QStringList, altSubjectMatches);
    QFETCH(QByteArray, clientCert);
    QFETCH(QString, phase1PeapVer);
    QFETCH(QString, phase1PeapLabel);
    QFETCH(QString, phase1FastProvisioning);
    QFETCH(QString, phase2Auth);
    QFETCH(QString, phase2EapAuth);
    QFETCH(QByteArray, phase2CaCert);
    QFETCH(QString, phase2CaPath);
    QFETCH(QString, phase2SubjectMatch);
    QFETCH(QStringList, phase2AltSubjectMatches);
    QFETCH(QByteArray, phase2ClientCert);
    QFETCH(QString, password);
    QFETCH(quint32, passwordFlags);
    QFETCH(QByteArray, passwordRaw);
    QFETCH(quint32, passwordRawFlags);
    QFETCH(QByteArray, privateKey);
    QFETCH(QString, privateKeyPassword);
    QFETCH(quint32, privateKeyPasswordFlags);
    QFETCH(QByteArray, phase2PrivateKey);
    QFETCH(QString, phase2PrivateKeyPassword);
    QFETCH(quint32, phase2PrivateKeyPasswordFlags);
    QFETCH(QString, pin);
    QFETCH(quint32, pinFlags);
    QFETCH(bool, systemCaCerts);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_802_1X_EAP), eap);
    map.insert(QLatin1String(NM_SETTING_802_1X_IDENTITY), identity);
    map.insert(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY), anonymousIdentity);
    map.insert(QLatin1String(NM_SETTING_802_1X_PAC_FILE), pacFile);
    map.insert(QLatin1String(NM_SETTING_802_1X_CA_CERT), caCert);
    map.insert(QLatin1String(NM_SETTING_802_1X_DOMAIN_SUFFIX_MATCH), domainSuffixMatch);
    map.insert(QLatin1String(NM_SETTING_802_1X_CA_PATH), caPath);
    map.insert(QLatin1String(NM_SETTING_802_1X_SUBJECT_MATCH), subjectMatch);
    map.insert(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES), altSubjectMatches);
    map.insert(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT), clientCert);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), phase1PeapVer);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL), phase1PeapLabel);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING), phase1FastProvisioning);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH), phase2Auth);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), phase2EapAuth);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT), phase2CaCert);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH), phase2CaPath);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_SUBJECT_MATCH), phase2SubjectMatch);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_ALTSUBJECT_MATCHES), phase2AltSubjectMatches);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT), phase2ClientCert);
    map.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD), password);
    map.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS), passwordFlags);
    map.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW), passwordRaw);
    map.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS), passwordRawFlags);
    map.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY), privateKey);
    map.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD), privateKeyPassword);
    map.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS), privateKeyPasswordFlags);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY), phase2PrivateKey);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD), phase2PrivateKeyPassword);
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS), phase2PrivateKeyPasswordFlags);
    map.insert(QLatin1String(NM_SETTING_802_1X_PIN), pin);
    map.insert(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS), pinFlags);
    map.insert(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS), systemCaCerts);

    NetworkManager::Security8021xSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(Security8021xSettingTest)

#include "moc_8021xsettingtest.cpp"
