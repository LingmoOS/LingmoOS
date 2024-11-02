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
#ifndef KYENTERPRICESETTINGINFO_H
#define KYENTERPRICESETTINGINFO_H

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>

#include "lingmonetworkresourcemanager.h"
#include <QObject>

enum KyEapMethodType {
    TLS = 0,
    PEAP,
    TTLS,
    LEAP,
    PWD,
    FAST
};

class KyEapMethodTlsInfo
{
public:
    QString identity;
    QString domain;
    QString devIfaceName;
    QString caCertPath;
    bool    bNeedCa;
    QString clientCertPath;
    QString clientPrivateKey;
    QString clientPrivateKeyPWD;
    NetworkManager::Setting::SecretFlags m_privateKeyPWDFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodTlsInfo& info) const
    {
        if (this->identity == info.identity
                && this->domain == info.domain
//                && this->devIfaceName == info.devIfaceName
                && this->caCertPath == info.caCertPath
                && this->bNeedCa == info.bNeedCa
                && this->clientCertPath == info.clientCertPath
                && this->clientPrivateKey == info.clientPrivateKey
                && this->clientPrivateKeyPWD == info.clientPrivateKeyPWD
                && this->m_privateKeyPWDFlag == info.m_privateKeyPWDFlag) {
            return true;
        } else {
            return false;
        }
    }
};

typedef enum {
    KyAuthEapMethodUnknown = 0,
    KyAuthEapMethodMd5,
    KyAuthEapMethodMschapv2,
    KyAuthEapMethodOtp,
    KyAuthEapMethodGtc,
    KyAuthEapMethodTls
} KyEapMethodAuth;

typedef enum{
    KyAuthMethodUnknown = 0,
    KyAuthMethodPap,
    KyAuthMethodChap,
    KyAuthMethodMschap,
    KyAuthMethodMschapv2,
    KyAuthMethodGtc,
    KyAuthMethodOtp,
    KyAuthMethodMd5,
    KyAuthMethodTls
} KyNoEapMethodAuth;


class KyEapMethodPeapInfo
{
public:
    KyNoEapMethodAuth phase2AuthMethod;
    QString userName;
    QString userPWD;
    NetworkManager::Setting::SecretFlags m_passwdFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodPeapInfo& info) const
    {
        if (this->phase2AuthMethod == info.phase2AuthMethod
                && this->userName == info.userName
                && this->userPWD == info.userPWD
                && this->m_passwdFlag == info.m_passwdFlag) {
            return true;
        } else {
            return false;
        }
    }
};

enum KyTtlsAuthMethod
{
    AUTH_EAP,
    AUTH_NO_EAP
};

class KyEapMethodTtlsInfo
{
public:
    KyTtlsAuthMethod authType;
    KyEapMethodAuth authEapMethod;
    KyNoEapMethodAuth authNoEapMethod;
    QString userName;
    QString userPWD;
    NetworkManager::Setting::SecretFlags m_passwdFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodTtlsInfo& info) const
    {
        if (this->authType == info.authType) {
            if (authType == AUTH_EAP) {
               if (this->authEapMethod == info.authEapMethod
                && this ->userName == info.userName
                && this->userPWD == info.userPWD
                && this->m_passwdFlag == info.m_passwdFlag) {
                    return true;
                }
            } else {
                if (authType == AUTH_EAP) {
                   if (this->authNoEapMethod == info.authNoEapMethod
                    && this ->userName == info.userName
                    && this->userPWD == info.userPWD
                    && this->m_passwdFlag == info.m_passwdFlag) {
                        return true;
                    }
                }
            }

        }
        return false;
    }
};

typedef enum {
    KyFastProvisioningUnknown = -1,
    KyFastProvisioningDisabled,
    KyFastProvisioningAllowUnauthenticated,
    KyFastProvisioningAllowAuthenticated,
    KyFastProvisioningAllowBoth
}KyFastProvisioning;

class KyEapMethodLeapInfo
{
public:
    QString m_userName;
    QString m_userPwd;
    NetworkManager::Setting::SecretFlags m_passwdFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodLeapInfo& info) const
    {
        if (this->m_userName == info.m_userName
                && this->m_userPwd == info.m_userPwd
                && this->m_passwdFlag == info.m_passwdFlag) {
            return true;
        } else {
            return false;
        }
    }
};

class KyEapMethodPwdInfo
{
public:
    QString m_userName;
    QString m_userPwd;
    NetworkManager::Setting::SecretFlags m_passwdFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodPwdInfo& info) const
    {
        if (this->m_userName == info.m_userName
                && this->m_userPwd == info.m_userPwd
                && this->m_passwdFlag == info.m_passwdFlag) {
            return true;
        } else {
            return false;
        }
    }
};

class KyEapMethodFastInfo
{
public:
    QString m_anonIdentity;
    KyFastProvisioning m_pacProvisioning;
    bool m_allowAutoPacFlag;
    QString m_pacFilePath;
    KyNoEapMethodAuth m_authMethod;
    QString m_userName;
    QString m_userPwd;
    NetworkManager::Setting::SecretFlags m_passwdFlag;
    // only valid when update
    bool    bChanged;

    inline bool operator == (const KyEapMethodFastInfo& info) const
    {
        if (this->m_anonIdentity == info.m_anonIdentity
                && this->m_pacProvisioning == info.m_pacProvisioning
                && this->m_allowAutoPacFlag == info.m_allowAutoPacFlag
                && this->m_pacFilePath == info.m_pacFilePath
                && this->m_authMethod == info.m_authMethod
                && this->m_userName == info.m_userName
                && this->m_userPwd == info.m_userPwd
                && this->m_passwdFlag == info.m_passwdFlag) {
            return true;
        } else {
            return false;
        }
    }
};

void assembleEapMethodTlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTlsInfo &tlsInfo);
void assembleEapMethodPeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPeapInfo &peapInfo);
void assembleEapMethodTtlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTtlsInfo &ttlsInfo);
void assembleEapMethodLeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodLeapInfo &leapInfo);
void assembleEapMethodPwdSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPwdInfo &pwdInfo);
void assembleEapMethodFastSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodFastInfo &fastInfo);

void modifyEapMethodTlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTlsInfo &tlsInfo);
void modifyEapMethodPeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPeapInfo &peapInfo);
void modifyEapMethodTtlsSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodTtlsInfo &ttlsInfo);
void modifyEapMethodLeapSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodLeapInfo &leapInfo);
void modifyEapMethodPwdSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodPwdInfo &pwdInfo);
void modifyEapMethodFastSettings(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyEapMethodFastInfo &fastInfo);

#endif // KYENTERPRICESETTINGINFO_H
