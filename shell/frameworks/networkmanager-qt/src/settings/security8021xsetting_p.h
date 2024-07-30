/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SECURITY8021X_SETTING_P_H
#define NETWORKMANAGERQT_SECURITY8021X_SETTING_P_H

#include <QStringList>

namespace NetworkManager
{
class Security8021xSettingPrivate
{
public:
    Security8021xSettingPrivate();

    QString name;
    QList<Security8021xSetting::EapMethod> eap;
    QString identity;
    QString anonymousIdentity;
    QString domainSuffixMatch;
    QString pacFile;
    QByteArray caCert;
    QString caPath;
    QString subjectMatch;
    QStringList altSubjectMatches;
    QByteArray clientCert;
    Security8021xSetting::PeapVersion phase1PeapVer;
    Security8021xSetting::PeapLabel phase1PeapLabel;
    Security8021xSetting::FastProvisioning phase1FastProvisioning;
    Security8021xSetting::AuthMethod phase2AuthMethod;
    Security8021xSetting::AuthEapMethod phase2AuthEapMethod;
    QByteArray phase2CaCert;
    QString phase2CaPath;
    QString phase2SubjectMatch;
    QStringList phase2AltSubjectMatches;
    QByteArray phase2ClientCert;
    QString password;
    Setting::SecretFlags passwordFlags;
    QByteArray passwordRaw;
    Setting::SecretFlags passwordRawFlags;
    QByteArray privateKey;
    QString privateKeyPassword;
    Setting::SecretFlags privateKeyPasswordFlags;
    QByteArray phase2PrivateKey;
    QString phase2PrivateKeyPassword;
    Setting::SecretFlags phase2PrivateKeyPasswordFlags;
    QString pin;
    Setting::SecretFlags pinFlags;
    bool systemCaCerts;
};

}

#endif // NETWORKMANAGERQT_SECURITY802X_SETTING_P_H
