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
#ifndef VPNADVANCEDPAGE_H
#define VPNADVANCEDPAGE_H

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include "kylable.h"
#include "kwidget.h"
#include "kpasswordedit.h"
#include "vpnconfigpage.h"
#include "kyvpnconnectoperation.h"

using namespace kdk;

enum TlsMode {
    NONE = 0,
    TLS_CERT,
    TLS_ENCRYPTION
};

enum ProxyType {
    NO = 0,
    HTTP,
    SOCKS
};

class VpnAdvancedPage : public QFrame
{
    Q_OBJECT
public:
    VpnAdvancedPage(QWidget *parent = nullptr);
    ~VpnAdvancedPage() = default;

    //Adia: Advanced
    void setL2tpOrPptpAdiaInfo(const KyVpnConfig &vpnInfo);
    void setOpenVpnAdiaInfo(const KyVpnConfig &vpnInfo);
    void setStrongSwanAdiaInfo(const KyVpnConfig &vpnInfo);

    bool checkIsChanged(const KyVpnConfig &vpnInfo, VpnType &vpnType);
    bool checkL2tpIsChanged(const KyVpnConfig &vpnInfo);
    bool checkPptpIsChanged(const KyVpnConfig &vpnInfo);
    bool checkOpenVpnIsChanged(const KyVpnConfig &vpnInfo);
    bool checkStrongSwanIsChanged(const KyVpnConfig &vpnInfo);

    void updateL2tpOrPptpAdiaInfo(KyVpnConfig &vpnInfo);
    void updateOpenVpnAdiaInfo(KyVpnConfig &vpnInfo);
    void updateStrongSwanAdiaInfo(KyVpnConfig &vpnInfo);

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool        m_isOpenVpn;

    QFrame      *m_defaultAdiaFrame;
    QFrame      *m_mruFrame;
    QFrame      *m_openVpnAdiaFrame;
    QFrame      *m_sSwanAdiaFrame;

    //L2TP PPTP Encryption(E)
    QCheckBox   *m_mppeECheckbox = nullptr;
    QCheckBox   *m_useStatefulECheckbox = nullptr;
    QCheckBox   *m_sendPppEchoPkgCheckbox = nullptr;

    QLabel      *m_authModeLabel;
    QLabel      *m_compModeLabel;
    QLabel      *m_mruLabel;
    QLabel      *m_mtuLabel;
    QLineEdit   *m_mruEdit;
    QLineEdit   *m_mtuEdit;
    QComboBox   *m_mppeECombox = nullptr;
    //认证方式
    QListWidget *m_authModeListWidget = nullptr;
    QCheckBox   *m_papAuthCheckbox = nullptr;
    QCheckBox   *m_chapAuthCheckbox = nullptr;
    QCheckBox   *m_mschapAuthCheckbox = nullptr;
    QCheckBox   *m_mschap2AuthCheckbox = nullptr;
    QCheckBox   *m_eapAuthCheckbox = nullptr;
    //压缩方式
    QListWidget *m_compModeListWidget = nullptr;
    QCheckBox   *m_bsdCompModeCheckbox = nullptr;
    QCheckBox   *m_defaultCompModeCheckbox = nullptr;
    QCheckBox   *m_tcpCompModeCheckbox = nullptr;
    QCheckBox   *m_protocolCompModeCheckbox = nullptr;
    QCheckBox   *m_addressCompModeCheckbox = nullptr;

    //Open VPN
    //general
    QCheckBox   *m_customGatewayPortCheckbox = nullptr;
    QCheckBox   *m_customRenegotiaInrCheckbox = nullptr;
    QCheckBox   *m_compressionCheckbox = nullptr;
    QCheckBox   *m_tcpConnCheckbox = nullptr;
    QCheckBox   *m_setVDevTypeCheckbox = nullptr;
    QCheckBox   *m_setVDevNameCheckbox = nullptr;
    QCheckBox   *m_customMtuCheckbox = nullptr;
    QCheckBox   *m_customUdpFragSizeCheckbox = nullptr;
    QCheckBox   *m_tcpMssCheckbox = nullptr;
    QCheckBox   *m_randomRemoteHostCheckbox = nullptr;
    QCheckBox   *m_ipv6TunLinkCheckbox = nullptr;
    QCheckBox   *m_specPingInrCheckbox = nullptr;
    QCheckBox   *m_specExitPingCheckbox = nullptr;
    QCheckBox   *m_acceptAuthedPaksCheckbox = nullptr;
    QCheckBox   *m_specMaxRouteCheckbox = nullptr;

    QLineEdit   *m_gatewayPortEdit;
    QLineEdit   *m_renogotiaInrEdit;
    QLineEdit   *m_setVDevNameEdit;
    QLineEdit   *m_customMtuEdit;
    QLineEdit   *m_customUdpFragSizeEdit;
    QLineEdit   *m_specPingInrEdit;
    QLineEdit   *m_specExRePingEdit;
    QLineEdit   *m_specMaxRouteEdit;
    QComboBox   *m_compressionCombox = nullptr;
    QComboBox   *m_setVDevTypeCombox = nullptr;
    QComboBox   *m_specExitRestarCombox = nullptr;
    FixLabel    *m_customRenoInrLabel;
    FixLabel    *m_customMtuLabel;
    FixLabel    *m_customUdpLabel;
    FixLabel    *m_acceptAuthedPaksLabel;
    //TLS settings
    QLabel      *m_subjectMatchLabel;
    QLabel      *m_keyPathLabel;
    QLabel      *m_keyDirectionLabel;
    QLineEdit   *m_subjectMatchEdit;
    QLineEdit   *m_keyPathEdit;
    QPushButton *m_keyPathChooseBtn;
    QCheckBox   *m_usePreviousCertCheckbox = nullptr;
    QCheckBox   *m_verifyPeerCertCheckbox = nullptr;
    QComboBox   *m_serverCertCheckCombox = nullptr;
    QComboBox   *m_usePreviousCertCombox = nullptr;
    QComboBox   *m_verifyPeerCertCombox = nullptr;
    QComboBox   *m_tlsModeCombox = nullptr;
    QComboBox   *m_keyDirectionCombox = nullptr;
    //proxies
    QLabel      *m_proxyServerAddLabel;
    QLabel      *m_proxyPortLabel;
    QLabel      *m_proxyUsernameLabel;
    QLabel      *m_proxyPwdLabel;
    QComboBox   *m_proxyTypeCombox = nullptr;
    QCheckBox   *m_infiniteRetryCheckbox = nullptr;
    QLineEdit   *m_proxyServerAddEdit;
    QLineEdit   *m_proxyPortEdit;
    QLineEdit   *m_proxyUsernameEdit;
    KPasswordEdit *m_proxyPwdEdit;
    //security
    QCheckBox   *m_customKeySizeCheckbox = nullptr;
    QLineEdit   *m_customKeySizeEdit;
    QComboBox   *m_hmacAuthCombox = nullptr;

    //strongswan
    QListWidget *m_optionsListWidget = nullptr;
    QCheckBox   *m_requestInIPCheckbox = nullptr;
    QCheckBox   *m_udpEncapCheckbox = nullptr;
    QCheckBox   *m_ipCompCheckbox = nullptr;
    QCheckBox   *m_enablCustomCheckbox = nullptr;
    QFrame      *m_ikeEspFrame;
    QLineEdit   *m_ikeEdit;
    QLineEdit   *m_espEdit;

    void initUi();
    void initConnect();
    void initDefaultAdiaFrame(); //L2TP PPTP
    void initOpenVpnAdiaFrame(); //Open VPN
    void initSSwanAdiaFrame(); //strong-swan
    void initDefalutCheckState();
    void addListItem(QListWidget *listWidget, QWidget *widget);
    void initCheckWidget(QCheckBox *checkBox, FixLabel *label, QWidget *widget);

    void showL2tpAdiaPage();
    void showPptpAdiaPage();
    void showOpenVpnAdiaPage();
    void showSSwanAdiaPage();
    bool checkConfirmBtnIsEnabled();
    bool getTextEditState(QString text);

private Q_SLOTS:
    void onProxyTypeComboxIndexChanged();
    void onTlsModeComboxIndexChanged();
    void setEnableOfConfirmBtn();
    void onKeyPathButtonClicked();

public Q_SLOTS:
    void setVpnAdvancedPage(const VpnType &type);

Q_SIGNALS:
    void setAdvancedPageState(bool);
};

#endif // VPNADVANCEDPAGE_H
