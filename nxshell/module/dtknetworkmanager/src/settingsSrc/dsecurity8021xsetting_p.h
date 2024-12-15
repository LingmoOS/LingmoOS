// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSECURITY8021XSETTING_P_H
#define DSECURITY8021XSETTING_P_H

#include "dsecurity8021xsetting.h"
#include "dnetworkmanager_global.h"
#include <QString>
#include <QList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSecurity8021xSettingPrivate
{
public:
    DSecurity8021xSettingPrivate();

    bool m_systemCaCerts;
    DSecurity8021xSetting::PeapVersion m_phase1PeapVer;
    DSecurity8021xSetting::PeapLabel m_phase1PeapLabel;
    DSecurity8021xSetting::FastProvisioning m_phase1FastProvisioning;
    DSecurity8021xSetting::AuthMethod m_phase2AuthMethod;
    DSecurity8021xSetting::AuthEapMethod m_phase2AuthEapMethod;
    DNMSetting::SecretFlags m_pinFlags;
    DNMSetting::SecretFlags m_phase2PrivateKeyPasswordFlags;
    DNMSetting::SecretFlags m_privateKeyPasswordFlags;
    DNMSetting::SecretFlags m_passwordRawFlags;
    DNMSetting::SecretFlags m_passwordFlags;
    QByteArray m_caCert;
    QByteArray m_clientCert;
    QByteArray m_phase2CaCert;
    QByteArray m_phase2ClientCert;
    QByteArray m_passwordRaw;
    QByteArray m_privateKey;
    QByteArray m_phase2PrivateKey;
    QString m_phase2PrivateKeyPassword;
    QString m_pin;
    QString m_privateKeyPassword;
    QString m_password;
    QString m_phase2CaPath;
    QString m_phase2DomainSuffixMatch;
    QString m_caPath;
    QString m_name;
    QString m_identity;
    QString m_anonymousIdentity;
    QString m_domainSuffixMatch;
    QString m_pacFile;
    QList<DSecurity8021xSetting::EapMethod> m_eap;
    QStringList m_altSubjectMatches;
    QStringList m_phase2AltSubjectMatches;
};

DNETWORKMANAGER_END_NAMESPACE

#endif
