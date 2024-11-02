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
#ifndef KYWIRELESSCONNECTOPERATION_H
#define KYWIRELESSCONNECTOPERATION_H

#include <QObject>
#include <QGSettings>
#include <QTimer>
#include "kyenterpricesettinginfo.h"
#include "lingmoconnectoperation.h"
#include "kyenterpricesettinginfo.h"

#define KEY_802_11_WIRELESS "802-11-wireless"
#define KEY_BLACKLIST_HOSTNAME "blacklist-hostname"

const QByteArray GSETTINGS_SCHEMA = "org.lingmo.lingmo-nm.switch";
const QString    WIRELESS_SWITCH  = "wirelessswitch";

const QString    WIFI_BAND_2_4GHZ = "2.4GHz";
const QString    WIFI_BAND_5GHZ   = "5GHz";

enum KySecuType {
    NONE = 0,
    WPA_AND_WPA2_PERSONAL,
    WPA_AND_WPA2_ENTERPRISE,
    WPA3_PERSONAL,
    WPA_AND_WPA3,
};

enum KyKeyMgmt {
    Unknown = -1,
    Wep, Ieee8021x,
    WpaNone,
    WpaPsk,
    WpaEap,
    SAE
};

class KyWirelessConnectSetting : public KyConnectSetting
{
//    Q_OBJECT

public:
    QString m_ssid;
    bool isAutoConnect;
    QString m_psk;
    NetworkManager::Setting::SecretFlags m_secretFlag;
    KyKeyMgmt m_type;
    //only if m_type == WpaEap
    KyEapMethodType m_eapMethodType;
};


class KyWirelessConnectOperation : public KyConnectOperation
{
    Q_OBJECT
public:
    explicit KyWirelessConnectOperation(QObject *parent = nullptr);
    ~KyWirelessConnectOperation();

    //无线开关
    void setWirelessEnabled(bool enabled);
    bool getWirelessEnabled();

    //获取密码保存策略
    bool getConnSecretFlags(QString &connUuid, NetworkManager::Setting::SecretFlags &);

    //获取KeyMgmt
    KyKeyMgmt getConnectKeyMgmt(const QString &uuid);

    //获取企业网类型
    bool getEnterpiseEapMethod(const QString &uuid, KyEapMethodType &type);

    //激活连接
    void activeWirelessConnect(QString , QString);
    //断开连接
    void deActivateWirelessConnection(const QString activeConnectName, const QString &activeConnectUuid);
    //新增普通连接
    void addConnect(const KyWirelessConnectSetting &connSettingInfo);
    //新增TLS连接
    void addTlsConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodTlsInfo &tlsinfo);
    //新增PEAP连接
    void addPeapConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodPeapInfo &peapInfo);
    //新增TTLS连接
    void addTtlsConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodTtlsInfo &ttlsInfo);
    //新增LEAP连接
    void addLeapConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodLeapInfo &leapInfo);
    //新增PWD连接
    void addPwdConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodPwdInfo &pwdInfo);
    //新增FAST连接
    void addFastConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodFastInfo &fastInfo);
    //新增连接并激活(普通wifi)
    void addAndActiveWirelessConnect(QString & devIface,KyWirelessConnectSetting &connSettingInfo,bool isHidden);

    //新增连接并激活(企业wifi)
    void addAndActiveWirelessEnterPriseTlsConnect(KyEapMethodTlsInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                  QString & devIface, bool isHidden);
    void addAndActiveWirelessEnterPrisePeapConnect(KyEapMethodPeapInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                   QString & devIface, bool isHidden);
    void addAndActiveWirelessEnterPriseTtlsConnect(KyEapMethodTtlsInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                   QString & devIface, bool isHidden);
    void addAndActiveWirelessEnterPriseLeapConnect(KyEapMethodLeapInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                   QString & devIface, bool isHidden);
    void addAndActiveWirelessEnterPrisePwdConnect(KyEapMethodPwdInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                  QString & devIface, bool isHidden);
    void addAndActiveWirelessEnterPriseFastConnect(KyEapMethodFastInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                   QString & devIface, bool isHidden);
    //属性页 page1 AutoConnect
    void setWirelessAutoConnect(const QString &uuid, bool bAutoConnect);
    //属性页 page2 page3 ipv6
    void updateIpv4AndIpv6SettingInfo(const QString &uuid, const KyConnectSetting &connectSettingsInfo);
    //属性页 page4 wifi Security
    //连接修改(安全改为个人/None)
    void updateWirelessPersonalConnect(const QString &uuid, const KyWirelessConnectSetting &connSettingInfo, bool bPwdChange);
    //连接修改(安全改为改为企业)
    void updateWirelessEnterPriseTlsConnect(const QString &uuid, const KyEapMethodTlsInfo &tlsinfo);
    void updateWirelessEnterPrisePeapConnect(const QString &uuid, const KyEapMethodPeapInfo &peapInfo);
    void updateWirelessEnterPriseTtlsConnect(const QString &uuid, const KyEapMethodTtlsInfo &ttlsInfo);
    void updateWirelessEnterPriseLeapConnect(const QString &uuid, const KyEapMethodLeapInfo &leapInfo);
    void updateWirelessEnterPrisePwdConnect(const QString &uuid, const KyEapMethodPwdInfo &pwdInfo);
    void updateWirelessEnterPriseFastConnect(const QString &uuid, const KyEapMethodFastInfo &fastInfo);
    //忘记
    void deleteWirelessConnect(const QString &connectUuid);
    //获取密码
    QString getPsk(const QString &connectUuid);
    QString getPrivateKeyPassword(const QString &connectUuid);
    QString get8021xPassword(const QString &connectUuid);

    //申请扫描
    void requestWirelessScan();

    void activeWirelessAp(const QString apUuid, const QString apName,
                              const QString apPassword, const QString apDevice,
                              const QString wirelessBand);
    void deactiveWirelessAp(const QString apName, const QString apUuid);

private:
    NetworkManager::ConnectionSettings::Ptr createWirelessApSetting(const QString apSsid,
                                                                        const QString apPassword,
                                                                        const QString apDevice,
                                                                        const QString wirelessBand);
    void updateWirelessApSetting(NetworkManager::Connection::Ptr apConnectPtr,
                                                                  const QString apName,
                                                                  const QString apPassword,
                                                                  const QString apDevice,
                                                                  const QString wirelessBand);
    QStringList getBlackListHostName(QString apConnectPath);


Q_SIGNALS:
    void wifiEnabledChanged(bool);
    void enabledWirelessNetwork(bool enabled);
    void addAndActivateConnectionError(QString errorMessage);

private:
    NetworkManager::WirelessNetwork::Ptr checkWifiNetExist(QString ssid, QString devName);
    void updateWirelessSecu(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyWirelessConnectSetting &connSettingInfo, bool bPwdChange = false);
    void setIpv4AndIpv6Setting(NetworkManager::ConnectionSettings::Ptr connSetting, const KyConnectSetting &connSettingInfo);
    void setWirelessSecuWpaXEap(NetworkManager::ConnectionSettings::Ptr connSettingPtr);
    void activateApConnectionByUuid(const QString apUuid, const QString apDevice);
};

#endif // KYWIRELESSCONNECTOPERATION_H
