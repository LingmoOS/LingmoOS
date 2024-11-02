/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "kyenterpricesettinginfo.h"

void assembleEapMethodTlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTlsInfo &info)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodTls);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setIdentity(info.identity);
    if (!info.domain.isEmpty()){
        wifi_8021x_sett->setDomainSuffixMatch(info.domain);
    }
    if (info.bNeedCa) {
        QByteArray caCerEndWithNull("file://" + info.caCertPath.toUtf8() + '\0');
        wifi_8021x_sett->setCaCertificate(caCerEndWithNull);
    } else {
        QByteArray caCerEndWithNull("");
        wifi_8021x_sett->setCaCertificate(caCerEndWithNull);
    }
    QByteArray cliCertEndWithNull("file://" + info.clientCertPath.toUtf8() + '\0');
    wifi_8021x_sett->setClientCertificate(cliCertEndWithNull);
    QByteArray cliPriKeyEndWithNull("file://" + info.clientPrivateKey.toUtf8() + '\0');
    wifi_8021x_sett->setPrivateKey(cliPriKeyEndWithNull);
    wifi_8021x_sett->setPrivateKeyPassword(info.clientPrivateKeyPWD);
    wifi_8021x_sett->setPrivateKeyPasswordFlags(info.m_privateKeyPWDFlag);

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);

    return;
}

void assembleEapMethodPeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPeapInfo &info)
{
    qDebug() << "assembleEapMethodPeapSettings";

    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)info.phase2AuthMethod);
    wifi_8021x_sett->setIdentity(info.userName);
    wifi_8021x_sett->setPassword(info.userPWD);
    wifi_8021x_sett->setPasswordFlags(info.m_passwdFlag);


    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
    return;
}

void assembleEapMethodTtlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTtlsInfo &info)
{

    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodTtls);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    if (info.authType == KyTtlsAuthMethod::AUTH_EAP)
    {
        wifi_8021x_sett->setPhase2AuthEapMethod((NetworkManager::Security8021xSetting::AuthEapMethod)info.authEapMethod);//gtc md5 mschapv2 otp tls
    } else if (info.authType == KyTtlsAuthMethod::AUTH_NO_EAP)
    {
        wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)info.authNoEapMethod);//chap md5 mschapv2 pap gtc mschap otp tls
    }
    wifi_8021x_sett->setIdentity(info.userName);
    wifi_8021x_sett->setPassword(info.userPWD);
    wifi_8021x_sett->setPasswordFlags(info.m_passwdFlag);

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
    return;
}


void modifyEapMethodTlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTlsInfo &tlsInfo)
{
    NetworkManager::Security8021xSetting::Ptr setting = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    setting->setInitialized(true);

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodTls);
    setting->setEapMethods(list);
    setting->setIdentity(tlsInfo.identity);
    if(!tlsInfo.domain.isEmpty())
    {
        setting->setDomainSuffixMatch(tlsInfo.domain);
    }
    if (tlsInfo.bNeedCa)
    {
        QByteArray caCerEndWithNull("file://" + tlsInfo.caCertPath.toUtf8() + '\0');
        setting->setCaCertificate(caCerEndWithNull);
    } else {
        QByteArray caCerEndWithNull("");
        setting->setCaCertificate(caCerEndWithNull);
    }

    QByteArray cliCertEndWithNull("file://" + tlsInfo.clientCertPath.toUtf8() + '\0');
    setting->setClientCertificate(cliCertEndWithNull);
    QByteArray cliPriKeyEndWithNull("file://" + tlsInfo.clientPrivateKey.toUtf8() + '\0');
    setting->setPrivateKey(cliPriKeyEndWithNull);
    setting->setPrivateKeyPasswordFlags(tlsInfo.m_privateKeyPWDFlag);
    if(tlsInfo.bChanged)
    {
        setting->setPrivateKeyPassword(tlsInfo.clientPrivateKeyPWD);
    }
    return;
}

void modifyEapMethodPeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPeapInfo &peapInfo)
{
    qDebug() << "assembleEapMethodPeapSettings";

    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    wifi_8021x_sett->setInitialized(true);

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)peapInfo.phase2AuthMethod);
    wifi_8021x_sett->setIdentity(peapInfo.userName);
    if(peapInfo.bChanged)
    {
        wifi_8021x_sett->setPassword(peapInfo.userPWD);
    }
    wifi_8021x_sett->setPasswordFlags(peapInfo.m_passwdFlag);

    QByteArray caCerEndWithNull("");
    wifi_8021x_sett->setCaCertificate(caCerEndWithNull);

    return;
}

void modifyEapMethodTtlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTtlsInfo &ttlsInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodTtls);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    if (ttlsInfo.authType == KyTtlsAuthMethod::AUTH_EAP)
    {
        wifi_8021x_sett->setPhase2AuthEapMethod((NetworkManager::Security8021xSetting::AuthEapMethod)ttlsInfo.authEapMethod);//gtc md5 mschapv2 otp tls
    } else if (ttlsInfo.authType == KyTtlsAuthMethod::AUTH_NO_EAP)
    {
        wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)ttlsInfo.authNoEapMethod);//chap md5 mschapv2 pap gtc mschap otp tls
    }
    wifi_8021x_sett->setIdentity(ttlsInfo.userName);
    if(ttlsInfo.bChanged)
    {
        wifi_8021x_sett->setPassword(ttlsInfo.userPWD);
    }
    wifi_8021x_sett->setPasswordFlags(ttlsInfo.m_passwdFlag);

    QByteArray caCerEndWithNull("");
    wifi_8021x_sett->setCaCertificate(caCerEndWithNull);
    return;
}

void assembleEapMethodLeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodLeapInfo &leapInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodLeap);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setIdentity(leapInfo.m_userName);
    wifi_8021x_sett->setPassword(leapInfo.m_userPwd);
    wifi_8021x_sett->setPasswordFlags(leapInfo.m_passwdFlag);


    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
    return;
}

void assembleEapMethodPwdSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPwdInfo &pwdInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodPwd);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setIdentity(pwdInfo.m_userName);
    wifi_8021x_sett->setPassword(pwdInfo.m_userPwd);
    wifi_8021x_sett->setPasswordFlags(pwdInfo.m_passwdFlag);


    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
    return;
}

void assembleEapMethodFastSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodFastInfo &fastInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodFast);
    wifi_8021x_sett->setInitialized(true);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setAnonymousIdentity(fastInfo.m_anonIdentity);
    if (fastInfo.m_allowAutoPacFlag) {
        wifi_8021x_sett->setPhase1FastProvisioning((NetworkManager::Security8021xSetting::FastProvisioning)fastInfo.m_pacProvisioning);
    } else {
        wifi_8021x_sett->setPhase1FastProvisioning(NetworkManager::Security8021xSetting::FastProvisioning::FastProvisioningDisabled);
    }
    QByteArray pacEndWithNull("file://" + fastInfo.m_pacFilePath.toUtf8() + '\0');
    wifi_8021x_sett->setPacFile(pacEndWithNull);
    wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)fastInfo.m_authMethod);
    wifi_8021x_sett->setIdentity(fastInfo.m_userName);
    wifi_8021x_sett->setPassword(fastInfo.m_userPwd);
    wifi_8021x_sett->setPasswordFlags(fastInfo.m_passwdFlag);

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);

    return;
}

void modifyEapMethodLeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodLeapInfo &leapInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    wifi_8021x_sett->setInitialized(true);

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodLeap);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setIdentity(leapInfo.m_userName);
    if(leapInfo.bChanged)
    {
        wifi_8021x_sett->setPassword(leapInfo.m_userPwd);
    }
    wifi_8021x_sett->setPasswordFlags(leapInfo.m_passwdFlag);

    QByteArray caCerEndWithNull("");
    wifi_8021x_sett->setCaCertificate(caCerEndWithNull);

    return;
}

void modifyEapMethodPwdSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPwdInfo &pwdInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    wifi_8021x_sett->setInitialized(true);

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodPwd);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setIdentity(pwdInfo.m_userName);
    if(pwdInfo.bChanged)
    {
        wifi_8021x_sett->setPassword(pwdInfo.m_userPwd);
    }
    wifi_8021x_sett->setPasswordFlags(pwdInfo.m_passwdFlag);

    QByteArray caCerEndWithNull("");
    wifi_8021x_sett->setCaCertificate(caCerEndWithNull);

    return;
}

void modifyEapMethodFastSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodFastInfo &fastInfo)
{
    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connSettingPtr->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    wifi_8021x_sett->setInitialized(true);

    QList<NetworkManager::Security8021xSetting::EapMethod> list;
    list.append(NetworkManager::Security8021xSetting::EapMethod::EapMethodFast);
    wifi_8021x_sett->setEapMethods(list);
    wifi_8021x_sett->setAnonymousIdentity(fastInfo.m_anonIdentity);
    if (fastInfo.m_allowAutoPacFlag) {
        wifi_8021x_sett->setPhase1FastProvisioning((NetworkManager::Security8021xSetting::FastProvisioning)fastInfo.m_pacProvisioning);
    } else {
        wifi_8021x_sett->setPhase1FastProvisioning(NetworkManager::Security8021xSetting::FastProvisioning::FastProvisioningDisabled);
    }
    QByteArray pacEndWithNull("file://" + fastInfo.m_pacFilePath.toUtf8() + '\0');
    wifi_8021x_sett->setPacFile(pacEndWithNull);

    wifi_8021x_sett->setPhase2AuthMethod((NetworkManager::Security8021xSetting::AuthMethod)fastInfo.m_authMethod);
    wifi_8021x_sett->setIdentity(fastInfo.m_userName);
    if(fastInfo.bChanged)
    {
        wifi_8021x_sett->setPassword(fastInfo.m_userPwd);
    }
    wifi_8021x_sett->setPasswordFlags(fastInfo.m_passwdFlag);
    return;
}
