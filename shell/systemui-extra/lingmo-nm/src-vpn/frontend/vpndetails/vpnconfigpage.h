/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef VPNCONFIGPAGE_H
#define VPNCONFIGPAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "kylable.h"
#include "kwidget.h"
#include "kpasswordedit.h"
#include "kyvpnconnectoperation.h"

using namespace kdk;

#define KEY_DIRECTION_NONE_INDEX   0
#define KEY_DIRECTION_ZERO_INDEX   1
#define KEY_DIRECTION_ONE_INDEX    2

#define KEY_DIRECTION_NONE  "None"
#define KEY_DIRECTION_ZERO  "0"
#define KEY_DIRECTION_ONE   "1"

enum VpnType {
    UNKNOW = -1,
    L2TP = 0,
    OPEN_VPN,
    PPTP,
    STRONG_SWAN
};

enum OpenVpnAuthMethodIndex {
    CERTIFICATE_INDEX = 0,
    STATICPASSWD_INDEX,
    PASSWD_INDEX,
    CERTIFICATEANDPASSWD_INDEX,
};
enum StrongSwanAuthMethodIndex {
    KEY_INDEX = 0,
    AGENT_INDEX,
    SMARTCARD_INDEX,
    EAP_INDEX,
};

class VpnConfigPage : public QFrame
{
    Q_OBJECT
public:
    explicit VpnConfigPage(QWidget *parent = nullptr);
    ~VpnConfigPage() = default;

    VpnType getVpnType();
    void setVpnConfigInfo(const KyVpnConfig &vpnInfo);
    bool checkIsChanged(const KyVpnConfig &vpnInfo);

    void updateVpnConfigInfo(KyVpnConfig &vpnInfo);

private:
    QFrame      *m_vpnCommunalFrame;
    QFrame      *m_authModeFrame;
    QFrame      *m_caCertFrame;
    QFrame      *m_userCertFrame;
    QFrame      *m_staticKeysFrame;
    QFrame      *m_pinFrame;
    QFrame      *m_PrivateKeysFrame;
    QFrame      *m_usersFrame;
    QFrame      *m_userPwdOpFrame;
    QFrame      *m_ntFrame;

    //公有
    QLabel      *m_vpnTypeLabel;
    QLabel      *m_vpnNameLabel;
    FixLabel    *m_serverAddressLabel;
    QComboBox   *m_vpnTypeComboBox = nullptr;
    QLineEdit   *m_vpnNameEdit = nullptr;
    QLineEdit   *m_serverAddressEdit = nullptr;

    //认证方式 Authentication Mode
    FixLabel    *m_authModeLabel;
    QComboBox   *m_authModeComboBox = nullptr;

    //证书 私钥
    FixLabel    *m_caCertLabel;
    FixLabel    *m_userCertLabel;
    FixLabel    *m_privateKeyLabel;
    FixLabel    *m_privateKeyPwdLabel;
    FixLabel    *m_pwdOptionLabel;

    QLineEdit   *m_caCertPathEdit;
    QLineEdit   *m_userCertPathEdit;
    QLineEdit   *m_privateKeyEdit;
    KPasswordEdit *m_privateKeyPwdEdit;
    QComboBox   *m_pwdOptionCombox;

    QPushButton *m_caCertButton;
    QPushButton *m_userCertButton;
    QPushButton *m_privateKeyButton;

    //静态密钥
    QLabel      *m_staticKeyLabel;
    FixLabel    *m_keyDirectionLabel;
    QLabel      *m_noticesLabel;
    QLabel      *m_localIpLabel;
    QLabel      *m_remoteIpLabel;

    QLineEdit   *m_staticKeyPathEdit;
    QPushButton *m_staticKeyButton;
    QComboBox   *m_keyDirectionCombox;
    QLineEdit   *m_localIpEdit;
    QLineEdit   *m_remoteIpEdit;

    //智能卡 PIN码
    QLabel      *m_pinLabel;
    QLineEdit   *m_pinEdit;

    //密码
    FixLabel    *m_usernameLabel;
    QLabel      *m_userPwdLabel;
    FixLabel    *m_userPwdOptionLabel;
    QLineEdit   *m_usernameEdit;
    KPasswordEdit *m_userPwdEdit;
    QComboBox   *m_userPwdOptionCombox;

    //L2TP PPTP公有
    QLabel      *m_ntDomainLabel;
    QLineEdit   *m_ntDomainEdit;


    void initUI();
    void initConnect();
    void showL2tpPwd();
    void showPptpPwd();
    void showOpenVpnTls();
    void showOpenVpnPwd();
    void showOpenVpnPwdTls();
    void showOpenVpnStaticKey();
    void showSswanCertPrivteKey();
    void showSswanCertSsh();
    void showSswanSmartCard();
    void showSswanEap();

    bool getTextEditState(QString text);
    bool checkConfirmBtnIsEnabled();


    //填充VPN认证信息
    void setPwdAuthInfo(const QString &username, const QString &userPwd, const KyPasswdPolicy &userPwdPolicy);
    void setCaCertAuthInfo(const QString &caCertificate);

    void setKeyAuthInfo(const QString &userCert, const QString &privateKey,
                        const QString &privateKeyPwd, const KyPasswdPolicy &privatePwdPolicy);

    void setStaticKeyAuthInfo(const QString &staticKey, const QString &vpnKeyDirection,
                              const QString &localIp, const QString &remoteIp);

    void setAgentAuthInfo(const QString &userCert);
    void setSmartCardAuthInfo(const QString &pin);
    void setEapAuthInfo(const QString &username, const QString &userPwd);
    void setNtDomain(const QString &ntDomain);

    //set info
    void setL2tpConfigInfo(const KyVpnConfig &vpnInfo);
    void setPptpConfigInfo(const KyVpnConfig &vpnInfo);
    void setOpenVpnConfigInfo(const KyVpnConfig &vpnInfo);
    void setSSwanVpnConfigInfo(const KyVpnConfig &vpnInfo);

    //check change
    bool checkL2tpIsChanged(const KyVpnConfig &vpnInfo);
    bool checkPptpIsChanged(const KyVpnConfig &vpnInfo);
    bool checkOpenVpnIsChanged(const KyVpnConfig &vpnInfo);
    bool checkStrongSwanIsChanged(const KyVpnConfig &vpnInfo);

    //update
    void updateL2tpConfigInfo(KyVpnConfig &vpnInfo);
    void updatePptpConfigInfo(KyVpnConfig &vpnInfo);
    void updateOpenVpnConfigInfo(KyVpnConfig &vpnInfo);
    void updateSSwanVpnConfigInfo(KyVpnConfig &vpnInfo);

private Q_SLOTS:
    void onVpnTypeComboxIndexChanged();
    void onAuthModeComboxIndexChanged();
    void setEnableOfConfirmBtn();
    void onCaCertButtonClicked();
    void onUserCertButtonClicked();
    void onStaticKeyButtonClicked();
    void onPrivateKeyButtonClicked();

Q_SIGNALS:
    void setConfigPageState(bool);
    void vpnTypeChanged(VpnType type);
};

#endif // VPNCONFIGPAGE_H
