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
#include "vpnadvancedpage.h"

#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#define  NO_LAYOUT_MARGINS  0, 0, 0, 0
#define  NO_SPACE  0
#define  LAYOUT_SPACING_SMALL 8
#define  HOR_LAYOUT_SPACING 24
#define  LAYOUT_ADD_SPACING  16
#define  VLAYOUT_ADD_SPACING 24
#define  LISTWIDGET_HEIGHT   184
#define  LISTWIDGET_HEIGHT_SMALL  112
#define  MIN_EDIT_WIDTH   150
#define  FIX_EDIT_WIDTH   200
#define  MIN_LABEL_WIDTH  140
#define  MAX_LABEL_WIDTH  240
#define  ONE_LABEL_WIDTH  440
#define  MAX_MRU_LENGTH   4
#define  ITEM_HEIGHT      36
#define  COMP_MODE_PROTOCOL  3
#define  COMP_MODE_ADDRESS   4
#define  LOG_FLAG  "[VPN AdvancedPage]"

VpnAdvancedPage::VpnAdvancedPage(QWidget *parent) : QFrame(parent)
{
    initUi();
    initConnect();
    initDefalutCheckState();
}

void VpnAdvancedPage::setL2tpOrPptpAdiaInfo(const KyVpnConfig &vpnInfo)
{
    //L2TP PPTP共有
    m_mppeECheckbox->setChecked(vpnInfo.m_mppeEnable);
    if (m_mppeECheckbox->isChecked()) {
        m_mppeECombox->setCurrentIndex((int)vpnInfo.m_mppeEncryptMethod);
    } else {
        m_mppeECombox->setEnabled(false);
    }
    m_useStatefulECheckbox->setChecked(vpnInfo.m_isAllowStatefulEncryption);
    m_useStatefulECheckbox->setEnabled(vpnInfo.m_mppeEnable);
    m_papAuthCheckbox->setEnabled(!vpnInfo.m_mppeEnable);
    m_chapAuthCheckbox->setEnabled(!vpnInfo.m_mppeEnable);
    m_eapAuthCheckbox->setEnabled(!vpnInfo.m_mppeEnable);

    m_sendPppEchoPkgCheckbox->setChecked(vpnInfo.m_sendPppPackage);

    //Auth Methods
    m_papAuthCheckbox->setChecked(vpnInfo.m_authMethods.testFlag(KYAUTH2_PAP));
    m_chapAuthCheckbox->setChecked(vpnInfo.m_authMethods.testFlag(KYAUTH2_CHAP));
    m_mschapAuthCheckbox->setChecked(vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAP));
    m_mschap2AuthCheckbox->setChecked(vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAPV2));
    m_eapAuthCheckbox->setChecked(vpnInfo.m_authMethods.testFlag(KYAUTH2_EAP));

    //Compress Methods
    m_bsdCompModeCheckbox->setChecked(vpnInfo.m_compressMethods.testFlag(KYCOMP_BSD));
    m_defaultCompModeCheckbox->setChecked(vpnInfo.m_compressMethods.testFlag(KYCOMP_DEFLATE));
    m_tcpCompModeCheckbox->setChecked(vpnInfo.m_compressMethods.testFlag(KYCOMP_TCP));


    //L2TP
    if (vpnInfo.m_vpnType == KYVPNTYPE_L2TP) {
        m_protocolCompModeCheckbox->setChecked(vpnInfo.m_compressMethods.testFlag(KYCOMP_PROTO));
        m_addressCompModeCheckbox->setChecked(vpnInfo.m_compressMethods.testFlag(KYCOMP_ADDR));

        m_mruEdit->setText(vpnInfo.m_mru);
        m_mtuEdit->setText(vpnInfo.m_mtu);
    }
}

void VpnAdvancedPage::setOpenVpnAdiaInfo(const KyVpnConfig &vpnInfo)
{
    //General
    m_customGatewayPortCheckbox->setChecked(vpnInfo.m_useAssignPort);
    m_gatewayPortEdit->setText(vpnInfo.m_assignPort);

    m_customRenegotiaInrCheckbox->setChecked(vpnInfo.m_useRenegSeconds);
    m_renogotiaInrEdit->setText(vpnInfo.m_renegSeconds);

    m_compressionCheckbox->setChecked(vpnInfo.m_useCompress);
    m_compressionCombox->setCurrentIndex((int)vpnInfo.m_openvpnCompress);

    m_tcpConnCheckbox->setChecked(vpnInfo.m_useTcpLink);

    m_setVDevTypeCheckbox->setChecked(vpnInfo.m_setDevType);
    m_setVDevTypeCombox->setCurrentIndex((int)vpnInfo.m_devType);

    m_setVDevNameCheckbox->setChecked(vpnInfo.m_setDevName);
    m_setVDevNameEdit->setText(vpnInfo.m_devName);

    m_customMtuCheckbox->setChecked(vpnInfo.m_useTunnelMtu);
    m_customMtuEdit->setText(vpnInfo.m_tunnelMtu);

    m_customUdpFragSizeCheckbox->setChecked(vpnInfo.m_useFragmentSize);
    m_customUdpFragSizeEdit->setText(vpnInfo.m_fragmentSize);

    m_tcpMssCheckbox->setChecked(vpnInfo.m_mssfix);

    m_randomRemoteHostCheckbox->setChecked(vpnInfo.m_remoteRandom);

    m_ipv6TunLinkCheckbox->setChecked(vpnInfo.m_ipv6TunLink);

    m_specPingInrCheckbox->setChecked(vpnInfo.m_setPingCycle);
    m_specPingInrEdit->setText(vpnInfo.m_pingCycle);

    m_specExitPingCheckbox->setChecked(vpnInfo.m_usePingMethod);
    m_specExitRestarCombox->setCurrentIndex((int)vpnInfo.m_pingMethod);
    m_specExRePingEdit->setText(vpnInfo.m_pingMethodTime);

    m_acceptAuthedPaksCheckbox->setChecked(vpnInfo.m_float);

    m_specMaxRouteCheckbox->setChecked(vpnInfo.m_setMaxRoute);
    m_specMaxRouteEdit->setText(vpnInfo.m_maxRoute);


    //TLS settings
    m_serverCertCheckCombox->setCurrentIndex((int)vpnInfo.m_checkServerCa);

    m_subjectMatchEdit->setText(vpnInfo.m_verifyName);

    m_usePreviousCertCheckbox->setChecked(vpnInfo.m_useRemoteCertTls);
    m_usePreviousCertCombox->setCurrentIndex((int)vpnInfo.m_remoteCertType);

    m_verifyPeerCertCheckbox->setChecked(vpnInfo.m_useNsCertTls);
    m_verifyPeerCertCombox->setCurrentIndex((int)vpnInfo.m_nsCertType);

    m_tlsModeCombox->setCurrentIndex(vpnInfo.m_vpnTlsMode);
    m_keyPathEdit->setText(vpnInfo.m_vpnKeyFile);
    if (vpnInfo.m_vpnTlsMode == KYVPNTLS_AUTHENTICATION) {
        if (vpnInfo.m_vpnTlsTaDir == KEY_DIRECTION_ZERO) {
            m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_ZERO_INDEX);
        } else if (vpnInfo.m_vpnTlsTaDir == KEY_DIRECTION_ONE) {
            m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_ONE_INDEX);
        } else {
            m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_NONE_INDEX);
        }
    } else {
        m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_NONE_INDEX);
    }

    //Proxies
    m_proxyTypeCombox->setCurrentIndex((int)vpnInfo.m_vpnProxyType);
    m_proxyServerAddEdit->setText(vpnInfo.m_vpnProxyServer);
    m_proxyPortEdit->setText(vpnInfo.m_vpnProxyPort);
    m_infiniteRetryCheckbox->setChecked(vpnInfo.m_vpnProxyRetry);
    m_proxyUsernameEdit->setText(vpnInfo.m_vpnProxyName);
    m_proxyPwdEdit->setText(vpnInfo.m_vpnProxyPasswd);

    //Security
    m_customKeySizeCheckbox->setChecked(vpnInfo.m_useKeysize);
    m_customKeySizeEdit->setText(vpnInfo.m_keySize);
    m_hmacAuthCombox->setCurrentIndex((int)vpnInfo.m_hmacAuthMethod);
}

void VpnAdvancedPage::setStrongSwanAdiaInfo(const KyVpnConfig &vpnInfo)
{
    m_requestInIPCheckbox->setChecked(vpnInfo.m_virtual);
    m_udpEncapCheckbox->setChecked(vpnInfo.m_encap);
    m_ipCompCheckbox->setChecked(vpnInfo.m_ipcomp);
    m_enablCustomCheckbox->setChecked(vpnInfo.m_proposal);

    if (vpnInfo.m_proposal) {
        m_ikeEdit->setText(vpnInfo.m_ike);
        m_espEdit->setText(vpnInfo.m_esp);
    }
}

bool VpnAdvancedPage::checkIsChanged(const KyVpnConfig &vpnInfo, VpnType &vpnType)
{
    bool isChanged = false;
    switch (vpnType) {
    case L2TP:
        if (checkL2tpIsChanged(vpnInfo)) {
            isChanged = true;
        }
        break;
    case PPTP:
        if (checkPptpIsChanged(vpnInfo)) {
            isChanged = true;
        }
        break;
    case OPEN_VPN:
        if (checkOpenVpnIsChanged(vpnInfo)) {
            isChanged = true;
        }
        break;
    case STRONG_SWAN:
        if (checkStrongSwanIsChanged(vpnInfo)) {
            isChanged = true;
        }
        break;
    default:
        qDebug() << LOG_FLAG << "Unknow VPN Type" << vpnType;
        break;
    }
    return isChanged;
}

bool VpnAdvancedPage::checkL2tpIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    if (vpnInfo.m_mppeEnable != m_mppeECheckbox->isChecked()
            || m_mppeECombox->currentData().toInt() != vpnInfo.m_mppeEncryptMethod
            || vpnInfo.m_isAllowStatefulEncryption != m_useStatefulECheckbox->isChecked()
            || vpnInfo.m_sendPppPackage != m_sendPppEchoPkgCheckbox->isChecked()) {

        isChanged = true;
    }

    if (vpnInfo.m_authMethods.testFlag(KYAUTH2_PAP) != m_papAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_CHAP) != m_chapAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAP) != m_mschapAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAPV2) != m_mschap2AuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_EAP) != m_eapAuthCheckbox->isChecked()) {

        isChanged = true;
    }

    if (vpnInfo.m_compressMethods.testFlag(KYCOMP_BSD) != m_bsdCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_DEFLATE) != m_defaultCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_TCP) != m_tcpCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_PROTO) != m_protocolCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_ADDR) != m_addressCompModeCheckbox->isChecked()) {

        isChanged = true;
    }

    if (vpnInfo.m_mru != m_mruEdit->text() || vpnInfo.m_mtu != m_mtuEdit->text()) {
        isChanged = true;
    }

    return isChanged;
}

bool VpnAdvancedPage::checkPptpIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    if (vpnInfo.m_mppeEnable != m_mppeECheckbox->isChecked()
            || m_mppeECombox->currentData().toInt() != vpnInfo.m_mppeEncryptMethod
            || vpnInfo.m_isAllowStatefulEncryption != m_useStatefulECheckbox->isChecked()
            || vpnInfo.m_sendPppPackage != m_sendPppEchoPkgCheckbox->isChecked()) {

        isChanged = true;
    }

    if (vpnInfo.m_authMethods.testFlag(KYAUTH2_PAP) != m_papAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_CHAP) != m_chapAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAP) != m_mschapAuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_MSCHAPV2) != m_mschap2AuthCheckbox->isChecked()
            || vpnInfo.m_authMethods.testFlag(KYAUTH2_EAP) != m_eapAuthCheckbox->isChecked()) {

        isChanged = true;
    }

    if (vpnInfo.m_compressMethods.testFlag(KYCOMP_BSD) != m_bsdCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_DEFLATE) != m_defaultCompModeCheckbox->isChecked()
            || vpnInfo.m_compressMethods.testFlag(KYCOMP_TCP) != m_tcpCompModeCheckbox->isChecked()) {

        isChanged = true;
    }

     return isChanged;
}

bool VpnAdvancedPage::checkOpenVpnIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    //General
    if (vpnInfo.m_useAssignPort != m_customGatewayPortCheckbox->isChecked()
            || vpnInfo.m_assignPort != m_gatewayPortEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_useRenegSeconds != m_customRenegotiaInrCheckbox->isChecked()
            || vpnInfo.m_renegSeconds != m_renogotiaInrEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_useCompress != m_compressionCheckbox->isChecked()
            || vpnInfo.m_openvpnCompress != m_compressionCombox->currentData().toInt()) {
        isChanged = true;
    }

    if (vpnInfo.m_useTcpLink != m_tcpConnCheckbox->isChecked()) {
        isChanged = true;
    }

    if (vpnInfo.m_setDevType != m_setVDevTypeCheckbox->isChecked()
            || vpnInfo.m_devType != m_setVDevTypeCombox->currentData().toInt()) {
        isChanged = true;
    }

    if (vpnInfo.m_setDevName != m_setVDevNameCheckbox->isChecked()
            || vpnInfo.m_devName != m_setVDevNameEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_useTunnelMtu != m_customMtuCheckbox->isChecked()
            || vpnInfo.m_tunnelMtu != m_customMtuEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_useFragmentSize != m_customUdpFragSizeCheckbox->isChecked()
            || vpnInfo.m_fragmentSize != m_customUdpFragSizeEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_mssfix != m_tcpMssCheckbox->isChecked()) {
        isChanged = true;
    }

    if (vpnInfo.m_remoteRandom != m_randomRemoteHostCheckbox->isChecked()) {
        isChanged = true;
    }

    if (vpnInfo.m_ipv6TunLink != m_ipv6TunLinkCheckbox->isChecked()) {
        isChanged = true;
    }

    if (vpnInfo.m_setPingCycle != m_specPingInrCheckbox->isChecked()
            || vpnInfo.m_pingCycle != m_specPingInrEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_usePingMethod != m_specExitPingCheckbox->isChecked()
            || vpnInfo.m_pingMethod != m_specExitRestarCombox->currentData().toInt()
            || vpnInfo.m_pingMethodTime != m_specExRePingEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_float != m_acceptAuthedPaksCheckbox->isChecked()) {
        isChanged = true;
    }

    if (vpnInfo.m_setMaxRoute != m_specMaxRouteCheckbox->isChecked()
            || vpnInfo.m_maxRoute != m_specMaxRouteEdit->text()) {
        isChanged = true;
    }


    //TLS settings
    if (vpnInfo.m_checkServerCa != m_serverCertCheckCombox->currentData().toInt()) {
        isChanged = true;
    }

    if (vpnInfo.m_verifyName != m_subjectMatchEdit->text()) {
        isChanged = true;
    }

    if (vpnInfo.m_useRemoteCertTls != m_usePreviousCertCheckbox->isChecked()
            || vpnInfo.m_remoteCertType != m_usePreviousCertCombox->currentData().toInt()) {
        isChanged = true;
    }

    if (vpnInfo.m_useNsCertTls != m_verifyPeerCertCheckbox->isChecked()
            || vpnInfo.m_nsCertType != m_verifyPeerCertCombox->currentData().toInt()) {
        isChanged = true;
    }

    if (vpnInfo.m_vpnTlsMode != m_tlsModeCombox->currentData().toInt()
            || vpnInfo.m_vpnKeyFile != m_keyPathEdit->text()
            || vpnInfo.m_vpnTlsTaDir != m_keyDirectionCombox->currentData().toString()) {
        isChanged = true;
    }


    //Proxies
    int currentProxyType = m_proxyTypeCombox->currentData().toInt();
    if (vpnInfo.m_vpnProxyType != currentProxyType) {
        isChanged = true;
    }

    if (currentProxyType == HTTP || currentProxyType == SOCKS) {
        if (vpnInfo.m_vpnProxyServer != m_proxyServerAddEdit->text()
                || vpnInfo.m_vpnProxyPort != m_proxyPortEdit->text()
                || vpnInfo.m_vpnProxyRetry != m_infiniteRetryCheckbox->isChecked()) {
            isChanged = true;
        }

        if (currentProxyType == HTTP) {
            if (vpnInfo.m_vpnProxyName != m_proxyUsernameEdit->text()
                    || vpnInfo.m_vpnProxyPasswd != m_proxyPwdEdit->text()) {
                isChanged = true;
            }
        }
    }


    //Security
    if (vpnInfo.m_useKeysize != m_customKeySizeCheckbox->isChecked()
            || vpnInfo.m_keySize != m_customKeySizeEdit->text()
            || vpnInfo.m_hmacAuthMethod != m_hmacAuthCombox->currentData().toInt()) {
        isChanged = true;
    }


    return isChanged;
}

bool VpnAdvancedPage::checkStrongSwanIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    if (vpnInfo.m_virtual != m_requestInIPCheckbox->isChecked()
            || vpnInfo.m_encap != m_udpEncapCheckbox->isChecked()
            || vpnInfo.m_ipcomp != m_ipCompCheckbox->isChecked()
            || vpnInfo.m_proposal != m_enablCustomCheckbox->isChecked()) {

        isChanged = true;
    }

    if (m_enablCustomCheckbox->isChecked()) {
        if (vpnInfo.m_ike != m_ikeEdit->text() || vpnInfo.m_esp != m_espEdit->text()) {
            isChanged = true;
        }
    }

    return isChanged;
}

void VpnAdvancedPage::updateL2tpOrPptpAdiaInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_mppeEnable = m_mppeECheckbox->isChecked();
    vpnInfo.m_mppeEncryptMethod = (KyMPPEMethod)m_mppeECombox->currentData().toInt();
    vpnInfo.m_isAllowStatefulEncryption = m_useStatefulECheckbox->isChecked();
    vpnInfo.m_sendPppPackage = m_sendPppEchoPkgCheckbox->isChecked();

    vpnInfo.m_authMethods.setFlag(KYAUTH2_PAP, m_papAuthCheckbox->isChecked());
    vpnInfo.m_authMethods.setFlag(KYAUTH2_CHAP, m_chapAuthCheckbox->isChecked());
    vpnInfo.m_authMethods.setFlag(KYAUTH2_MSCHAP, m_mschapAuthCheckbox->isChecked());
    vpnInfo.m_authMethods.setFlag(KYAUTH2_MSCHAPV2, m_mschap2AuthCheckbox->isChecked());
    vpnInfo.m_authMethods.setFlag(KYAUTH2_EAP, m_eapAuthCheckbox->isChecked());

    vpnInfo.m_compressMethods.setFlag(KYCOMP_BSD, m_bsdCompModeCheckbox->isChecked());
    vpnInfo.m_compressMethods.setFlag(KYCOMP_DEFLATE, m_defaultCompModeCheckbox->isChecked());
    vpnInfo.m_compressMethods.setFlag(KYCOMP_TCP, m_tcpCompModeCheckbox->isChecked());

    if (!m_compModeListWidget->item(COMP_MODE_PROTOCOL)->isHidden()
            && !m_compModeListWidget->item(COMP_MODE_ADDRESS)->isHidden()) {
        vpnInfo.m_compressMethods.setFlag(KYCOMP_PROTO, m_protocolCompModeCheckbox->isChecked());
        vpnInfo.m_compressMethods.setFlag(KYCOMP_ADDR, m_addressCompModeCheckbox->isChecked());
        vpnInfo.m_mru = m_mruEdit->text();
        vpnInfo.m_mtu = m_mtuEdit->text();
        qDebug() << LOG_FLAG << "Update L2TP advanced info finished";

    } else {
        qDebug() << LOG_FLAG << "Update PPTP advanced info finished";
    }
}

void VpnAdvancedPage::updateOpenVpnAdiaInfo(KyVpnConfig &vpnInfo)
{
    //General
    vpnInfo.m_useAssignPort = m_customGatewayPortCheckbox->isChecked();
    vpnInfo.m_assignPort = m_gatewayPortEdit->text();

    vpnInfo.m_useRenegSeconds = m_customRenegotiaInrCheckbox->isChecked();
    vpnInfo.m_renegSeconds = m_renogotiaInrEdit->text();

    vpnInfo.m_useCompress = m_compressionCheckbox->isChecked();
    vpnInfo.m_openvpnCompress = (KyCompressMethod2)m_compressionCombox->currentData().toInt();

    vpnInfo.m_useTcpLink = m_tcpConnCheckbox->isChecked();

    vpnInfo.m_setDevType = m_setVDevTypeCheckbox->isChecked();
    vpnInfo.m_devType = (KyVirtualDeviceType)m_setVDevTypeCombox->currentData().toInt();

    vpnInfo.m_setDevName = m_setVDevNameCheckbox->isChecked();
    vpnInfo.m_devName = m_setVDevNameEdit->text();

    vpnInfo.m_useTunnelMtu = m_customMtuCheckbox->isChecked();
    vpnInfo.m_tunnelMtu = m_customMtuEdit->text();

    vpnInfo.m_useFragmentSize = m_customUdpFragSizeCheckbox->isChecked();
    vpnInfo.m_fragmentSize = m_customUdpFragSizeEdit->text();

    vpnInfo.m_mssfix = m_tcpMssCheckbox->isChecked();
    vpnInfo.m_remoteRandom = m_randomRemoteHostCheckbox->isChecked();
    vpnInfo.m_ipv6TunLink = m_ipv6TunLinkCheckbox->isChecked();

    vpnInfo.m_setPingCycle = m_specPingInrCheckbox->isChecked();
    vpnInfo.m_pingCycle = m_specPingInrEdit->text();

    vpnInfo.m_usePingMethod = m_specExitPingCheckbox->isChecked();
    vpnInfo.m_pingMethod = (KyVpnPingMethod)m_specExitRestarCombox->currentData().toInt();
    vpnInfo.m_pingMethodTime = m_specExRePingEdit->text();

    vpnInfo.m_float = m_acceptAuthedPaksCheckbox->isChecked();

    vpnInfo.m_setMaxRoute = m_specMaxRouteCheckbox->isChecked();
    vpnInfo.m_maxRoute = m_specMaxRouteEdit->text();


    //TLS settings
    vpnInfo.m_checkServerCa = (KyVpnCheckServerMethod)m_serverCertCheckCombox->currentData().toInt();
    vpnInfo.m_verifyName = m_subjectMatchEdit->text();

    vpnInfo.m_useRemoteCertTls = m_usePreviousCertCheckbox->isChecked();
    vpnInfo.m_remoteCertType = (KyVpnCertType)m_usePreviousCertCombox->currentData().toInt();

    vpnInfo.m_useNsCertTls = m_verifyPeerCertCheckbox->isChecked();
    vpnInfo.m_nsCertType = (KyVpnCertType)m_verifyPeerCertCombox->currentData().toInt();

    vpnInfo.m_vpnTlsMode = (KyVpnTlsMode)m_tlsModeCombox->currentData().toInt();
    vpnInfo.m_vpnKeyFile = m_keyPathEdit->text();
    vpnInfo.m_vpnTlsTaDir = m_keyDirectionCombox->currentData().toString();


    //Proxies
    vpnInfo.m_vpnProxyType = (KyVpnProxyType)m_proxyTypeCombox->currentData().toInt();
    vpnInfo.m_vpnProxyServer = m_proxyServerAddEdit->text();
    vpnInfo.m_vpnProxyPort = m_proxyPortEdit->text();
    vpnInfo.m_vpnProxyRetry = m_infiniteRetryCheckbox->isChecked();
    vpnInfo.m_vpnProxyName = m_proxyUsernameEdit->text();
    vpnInfo.m_vpnProxyPasswd = m_proxyPwdEdit->text();


    //Security
    vpnInfo.m_useKeysize = m_customKeySizeCheckbox->isChecked();
    vpnInfo.m_keySize = m_customKeySizeEdit->text();
    vpnInfo.m_hmacAuthMethod = (KyHMACAuthMethod)m_hmacAuthCombox->currentData().toInt();

    qDebug() << LOG_FLAG << "Update open VPN advanced info finished";
}

void VpnAdvancedPage::updateStrongSwanAdiaInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_virtual = m_requestInIPCheckbox->isChecked();
    vpnInfo.m_encap = m_udpEncapCheckbox->isChecked();
    vpnInfo.m_ipcomp = m_ipCompCheckbox->isChecked();
    vpnInfo.m_proposal = m_enablCustomCheckbox->isChecked();

    vpnInfo.m_ike = m_ikeEdit->text();
    vpnInfo.m_esp = m_espEdit->text();

    qDebug() << LOG_FLAG << "update strong-swan advanced info finished";
}

void VpnAdvancedPage::setVpnAdvancedPage(const VpnType &type)
{
    switch (type) {
    case L2TP:
        showL2tpAdiaPage();
        break;
    case OPEN_VPN:
        showOpenVpnAdiaPage();
        break;
    case PPTP:
        showPptpAdiaPage();
        break;
    case STRONG_SWAN:
        showSSwanAdiaPage();
        break;
    default:
        break;
    }
}

void VpnAdvancedPage::paintEvent(QPaintEvent *event)
{
    if (m_authModeListWidget) {
        QPalette mpal(m_authModeListWidget->palette());
        mpal.setColor(QPalette::Base, qApp->palette().base().color());
        mpal.setColor(QPalette::AlternateBase, qApp->palette().alternateBase().color());
        m_authModeListWidget->setBackgroundRole(QPalette::Base);
        m_authModeListWidget->setAlternatingRowColors(true);
        m_authModeListWidget->setPalette(mpal);
    }
    if (m_compModeListWidget) {
        QPalette mpal(m_compModeListWidget->palette());
        mpal.setColor(QPalette::Base, qApp->palette().base().color());
        mpal.setColor(QPalette::AlternateBase, qApp->palette().alternateBase().color());
        m_compModeListWidget->setBackgroundRole(QPalette::Base);
        m_compModeListWidget->setAlternatingRowColors(true);
        m_compModeListWidget->setPalette(mpal);
    }
    if (m_optionsListWidget) {
        QPalette mpal(m_optionsListWidget->palette());
        mpal.setColor(QPalette::Base, qApp->palette().base().color());
        mpal.setColor(QPalette::AlternateBase, qApp->palette().alternateBase().color());
        m_optionsListWidget->setBackgroundRole(QPalette::Base);
        m_optionsListWidget->setAlternatingRowColors(true);
        m_optionsListWidget->setPalette(mpal);
    }
}

void VpnAdvancedPage::initUi()
{
    m_defaultAdiaFrame = new QFrame(this);
    m_openVpnAdiaFrame = new QFrame(this);
    m_sSwanAdiaFrame = new QFrame(this);

    initDefaultAdiaFrame();
    initOpenVpnAdiaFrame();
    initSSwanAdiaFrame();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    mainLayout->setSpacing(NO_SPACE);
    mainLayout->addWidget(m_defaultAdiaFrame);
    mainLayout->addWidget(m_openVpnAdiaFrame);
    mainLayout->addWidget(m_sSwanAdiaFrame);
    mainLayout->addSpacing(3);
    mainLayout->addStretch();
}

void VpnAdvancedPage::initConnect()
{
    connect(m_mppeECheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_mppeECombox->setEnabled(state);
        m_useStatefulECheckbox->setEnabled(state);

        m_papAuthCheckbox->setEnabled(!state);
        m_chapAuthCheckbox->setEnabled(!state);
        m_eapAuthCheckbox->setEnabled(!state);
    });

    //strong-Swan
    connect(m_enablCustomCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_ikeEspFrame->setEnabled(state);
    });

    //open VPN General
    connect(m_customGatewayPortCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_gatewayPortEdit->setEnabled(state);
    });
    connect(m_customRenegotiaInrCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_renogotiaInrEdit->setEnabled(state);
    });
    connect(m_compressionCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_compressionCombox->setEnabled(state);
    });
    connect(m_compressionCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::CheckState::Unchecked) {
            m_compressionCombox->setCurrentIndex(KYCOMP2_LZODISABLE);
        }
    });
    connect(m_setVDevTypeCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_setVDevTypeCombox->setEnabled(state);
    });
    connect(m_setVDevNameCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_setVDevNameEdit->setEnabled(state);
    });
    connect(m_customMtuCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_customMtuEdit->setEnabled(state);
    });
    connect(m_customUdpFragSizeCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_customUdpFragSizeEdit->setEnabled(state);
    });
    connect(m_specPingInrCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_specPingInrEdit->setEnabled(state);
    });
    connect(m_specExitPingCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_specExitRestarCombox->setEnabled(state);
    });
    connect(m_specExitPingCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_specExRePingEdit->setEnabled(state);
    });
    connect(m_specMaxRouteCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_specMaxRouteEdit->setEnabled(state);
    });

    //open VPN TLS settings
    connect(m_serverCertCheckCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index == (int)KYCHECKSERVER_NONE) {
            m_subjectMatchEdit->setEnabled(false);
        } else {
            m_subjectMatchEdit->setEnabled(true);
        }
    });
    connect(m_tlsModeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VpnAdvancedPage::onTlsModeComboxIndexChanged);
    connect(m_usePreviousCertCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_usePreviousCertCombox->setEnabled(state);
    });
    connect(m_verifyPeerCertCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_verifyPeerCertCombox->setEnabled(state);
    });
    //open VPN proxies
    connect(m_proxyTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VpnAdvancedPage::onProxyTypeComboxIndexChanged);
    //open VPN security
    connect(m_customKeySizeCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        m_customKeySizeEdit->setEnabled(state);
    });

    connect(m_mruEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_mtuEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_gatewayPortEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_renogotiaInrEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_setVDevNameEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customMtuEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customUdpFragSizeEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specPingInrEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specExRePingEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_subjectMatchEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specMaxRouteEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_keyPathEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_proxyServerAddEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_proxyPortEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_proxyUsernameEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_proxyPwdEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customKeySizeEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_ikeEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_espEdit, &QLineEdit::textChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);

    connect(m_customGatewayPortCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customRenegotiaInrCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_setVDevNameCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customMtuCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customUdpFragSizeCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specPingInrCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specExitPingCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_specMaxRouteCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_customKeySizeCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);
    connect(m_enablCustomCheckbox, &QCheckBox::stateChanged, this, &VpnAdvancedPage::setEnableOfConfirmBtn);

    connect(m_serverCertCheckCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfConfirmBtn()));
    connect(m_tlsModeCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfConfirmBtn()));
    connect(m_proxyTypeCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfConfirmBtn()));
    connect(m_keyPathChooseBtn, &QPushButton::clicked, this, &VpnAdvancedPage::onKeyPathButtonClicked);
}

void VpnAdvancedPage::initDefaultAdiaFrame()
{
    /**********控件**********/
    m_mppeECheckbox = new QCheckBox(this);
    m_useStatefulECheckbox = new QCheckBox(this);
    m_sendPppEchoPkgCheckbox = new QCheckBox(this);
    m_authModeLabel = new QLabel(this);
    m_compModeLabel = new QLabel(this);
    m_mruLabel = new QLabel(this);
    m_mtuLabel = new QLabel(this);

    m_mppeECombox = new QComboBox(this);
    m_authModeListWidget = new QListWidget(this);
    m_papAuthCheckbox = new QCheckBox(this);
    m_chapAuthCheckbox = new QCheckBox(this);
    m_mschapAuthCheckbox = new QCheckBox(this);
    m_mschap2AuthCheckbox = new QCheckBox(this);
    m_eapAuthCheckbox = new QCheckBox(this);

    m_compModeListWidget = new QListWidget(this);
    m_bsdCompModeCheckbox = new QCheckBox(this);
    m_defaultCompModeCheckbox = new QCheckBox(this);
    m_tcpCompModeCheckbox = new QCheckBox(this);
    m_protocolCompModeCheckbox = new QCheckBox(this);
    m_addressCompModeCheckbox = new QCheckBox(this);

    m_mruFrame = new QFrame(this);
    m_mruEdit = new QLineEdit(this);
    m_mtuEdit = new QLineEdit(this);

    m_authModeListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_authModeListWidget->setFrameShape(QFrame::Shape::StyledPanel);
    addListItem(m_authModeListWidget, m_papAuthCheckbox);
    addListItem(m_authModeListWidget, m_chapAuthCheckbox);
    addListItem(m_authModeListWidget, m_mschapAuthCheckbox);
    addListItem(m_authModeListWidget, m_mschap2AuthCheckbox);
    addListItem(m_authModeListWidget, m_eapAuthCheckbox);

    m_compModeListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_compModeListWidget->setFrameShape(QFrame::Shape::StyledPanel);
    addListItem(m_compModeListWidget, m_bsdCompModeCheckbox);
    addListItem(m_compModeListWidget, m_defaultCompModeCheckbox);
    addListItem(m_compModeListWidget, m_tcpCompModeCheckbox);
    addListItem(m_compModeListWidget, m_protocolCompModeCheckbox);
    addListItem(m_compModeListWidget, m_addressCompModeCheckbox);

    /**********布局**********/
    QVBoxLayout *defaultAdiaVLayout = new QVBoxLayout(m_defaultAdiaFrame);
    defaultAdiaVLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    defaultAdiaVLayout->setSpacing(LAYOUT_SPACING_SMALL);

    QFrame *secuFrame= new QFrame(m_defaultAdiaFrame);
    QFormLayout *secuLayout = new QFormLayout(secuFrame);
    secuLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    secuLayout->setVerticalSpacing(LAYOUT_SPACING_SMALL);
    secuLayout->setHorizontalSpacing(HOR_LAYOUT_SPACING);
    secuLayout->addRow(m_mppeECheckbox, m_mppeECombox);
    secuLayout->addRow(m_useStatefulECheckbox);
    secuLayout->addRow(m_sendPppEchoPkgCheckbox);
    m_mppeECheckbox->setMinimumWidth(MIN_LABEL_WIDTH);

    QHBoxLayout *mruHLayout = new QHBoxLayout(m_mruFrame);
    mruHLayout->setContentsMargins(0, 16, 0, 0);
    mruHLayout->setSpacing(HOR_LAYOUT_SPACING);
    mruHLayout->addWidget(m_mruLabel);
    mruHLayout->addWidget(m_mruEdit);
    mruHLayout->addStretch();
    mruHLayout->addWidget(m_mtuLabel);
    mruHLayout->addWidget(m_mtuEdit);
    m_mruEdit->setFixedWidth(MIN_EDIT_WIDTH);
    m_mtuEdit->setFixedWidth(MIN_EDIT_WIDTH);

    defaultAdiaVLayout->addWidget(secuFrame);
    defaultAdiaVLayout->addSpacing(4);
    defaultAdiaVLayout->addWidget(m_authModeLabel);
    defaultAdiaVLayout->addWidget(m_authModeListWidget);
    defaultAdiaVLayout->addSpacing(LAYOUT_ADD_SPACING);
    defaultAdiaVLayout->addWidget(m_compModeLabel);
    defaultAdiaVLayout->addWidget(m_compModeListWidget);
    defaultAdiaVLayout->addWidget(m_mruFrame);
    m_authModeListWidget->setFixedHeight(LISTWIDGET_HEIGHT);
    m_compModeListWidget->setFixedHeight(LISTWIDGET_HEIGHT);

    /**********控件显示**********/
    m_mppeECheckbox->setText(tr("MPPE encryption algorithm:")); //MPPE加密算法：
    m_useStatefulECheckbox->setText(tr("Use Stateful encryption")); //使用有状态加密
    m_sendPppEchoPkgCheckbox->setText(tr("Send PPP echo packets")); //发送PPP回显包
    m_authModeLabel->setText(tr("Authentication Mode:")); //认证方式：
    m_papAuthCheckbox->setText(tr("PAP authentication")); //PAP认证
    m_chapAuthCheckbox->setText(tr("CHAP authentication")); //CHAP认证
    m_mschapAuthCheckbox->setText(tr("MSCHAP authentication")); //MSCHAP认证
    m_mschap2AuthCheckbox->setText(tr("MSCHAP2 authentication")); //MSCHAP2认证
    m_eapAuthCheckbox->setText(tr("EAP authentication")); //EAP认证
    m_compModeLabel->setText(tr("Compression Mode:")); //压缩方式：
    m_bsdCompModeCheckbox->setText(tr("Allow BSD data compression")); //允许BSD压缩
    m_defaultCompModeCheckbox->setText(tr("Allow Default data compression")); //允许Default压缩
    m_tcpCompModeCheckbox->setText(tr("Allow TCP header compression")); //允许TCP头压缩
    m_protocolCompModeCheckbox->setText(tr("Use protocol field compression negotiation")); //使用协议域压缩协商
    m_addressCompModeCheckbox->setText(tr("Use Address/Control compression")); //使用地址/控制压缩
    m_mruLabel->setText("MRU");
    m_mtuLabel->setText("MTU");
    m_mruEdit->setText("0");
    m_mtuEdit->setText("0");

    m_mppeECombox->addItem(tr("All Available"), KYMPPE_DEFAULT); //任意
    m_mppeECombox->addItem(tr("128-bit"), KYMPPE_REQUIRE128); //128位
    m_mppeECombox->addItem(tr("40-bit"), KYMPPE_REQUIRE40); //40位

    //控件输入格式限制
    QRegExp rxNumber("^[0-9]+$");
    m_mruEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_mtuEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_mruEdit->setMaxLength(MAX_MRU_LENGTH);
    m_mtuEdit->setMaxLength(MAX_MRU_LENGTH);
}

void VpnAdvancedPage::initOpenVpnAdiaFrame()
{
    /**********控件**********/
    //General
    QLabel *generalLabel = new QLabel(m_openVpnAdiaFrame);
    m_customGatewayPortCheckbox = new QCheckBox(this);
    m_customRenegotiaInrCheckbox = new QCheckBox(this);
    m_compressionCheckbox = new QCheckBox(this);
    m_tcpConnCheckbox = new QCheckBox(this);
    m_setVDevTypeCheckbox = new QCheckBox(this);
    m_setVDevNameCheckbox = new QCheckBox(this);
    m_customMtuCheckbox = new QCheckBox(this);
    m_customUdpFragSizeCheckbox = new QCheckBox(this);
    m_tcpMssCheckbox = new QCheckBox(this);
    m_randomRemoteHostCheckbox = new QCheckBox(this);
    m_ipv6TunLinkCheckbox = new QCheckBox(this);
    m_specPingInrCheckbox = new QCheckBox(this);
    m_specExitPingCheckbox = new QCheckBox(this);
    m_acceptAuthedPaksCheckbox = new QCheckBox(this);
    m_specMaxRouteCheckbox = new QCheckBox(this);

    m_gatewayPortEdit = new QLineEdit(this);
    m_renogotiaInrEdit = new QLineEdit(this);
    m_setVDevNameEdit = new QLineEdit(this);
    m_customMtuEdit = new QLineEdit(this);
    m_customUdpFragSizeEdit = new QLineEdit(this);
    m_specPingInrEdit = new QLineEdit(this);
    m_specExRePingEdit = new QLineEdit(this);
    m_specMaxRouteEdit = new QLineEdit(this);
    m_compressionCombox = new QComboBox(this);
    m_setVDevTypeCombox = new QComboBox(this);
    m_specExitRestarCombox = new QComboBox(this);
    m_customRenoInrLabel = new FixLabel(this);
    m_customMtuLabel = new FixLabel(this);
    m_acceptAuthedPaksLabel = new FixLabel(this);
    m_customUdpLabel = new FixLabel(this);

    //TLS settings
    QLabel *tlsSettingLabel = new QLabel(m_openVpnAdiaFrame);
    QLabel *serverCertCheckLabel = new QLabel(m_openVpnAdiaFrame);
    QLabel *tlsModeLabel = new QLabel(m_openVpnAdiaFrame);

    FixLabel *usePreviousCertLabel = new FixLabel(m_openVpnAdiaFrame);
    FixLabel *verifyPeerCertLabel = new FixLabel(m_openVpnAdiaFrame);

    m_subjectMatchLabel = new QLabel(this);
    m_keyPathLabel = new QLabel(this);
    m_keyDirectionLabel = new QLabel(this);
    m_subjectMatchEdit = new QLineEdit(this);
    m_keyPathEdit = new QLineEdit(this);
    m_keyPathChooseBtn = new QPushButton(this);
    m_usePreviousCertCheckbox = new QCheckBox(this);
    m_verifyPeerCertCheckbox = new QCheckBox(this);
    m_serverCertCheckCombox = new QComboBox(this);
    m_usePreviousCertCombox = new QComboBox(this);
    m_verifyPeerCertCombox = new QComboBox(this);
    m_tlsModeCombox = new QComboBox(this);
    m_keyDirectionCombox = new QComboBox(this);

    //Proxies
    QLabel *proxiesLabel = new QLabel(m_openVpnAdiaFrame);
    QLabel *proxyTypeLabel = new QLabel(m_openVpnAdiaFrame);
    m_proxyServerAddLabel = new QLabel(this);
    m_proxyPortLabel = new QLabel(this);
    m_proxyUsernameLabel = new QLabel(this);
    m_proxyPwdLabel = new QLabel(this);
    m_proxyTypeCombox = new QComboBox(this);
    m_infiniteRetryCheckbox = new QCheckBox(this);
    m_proxyServerAddEdit = new QLineEdit(this);
    m_proxyPortEdit = new QLineEdit(this);
    m_proxyUsernameEdit = new QLineEdit(this);
    m_proxyPwdEdit = new KPasswordEdit(this);
    m_proxyPwdEdit->setClearButtonEnabled(false);

    //Security
    QLabel *securityLabel = new QLabel(m_openVpnAdiaFrame);
    QLabel *hmacLabel = new QLabel(m_openVpnAdiaFrame);
    m_customKeySizeCheckbox = new QCheckBox(this);
    m_customKeySizeEdit = new QLineEdit(this);
    m_hmacAuthCombox = new QComboBox(this);


    /**********布局**********/
    QVBoxLayout *openVpnAdiaVLayout = new QVBoxLayout(m_openVpnAdiaFrame);
    openVpnAdiaVLayout->setContentsMargins(NO_LAYOUT_MARGINS);

    //General
    QFrame *generalFrame = new QFrame(m_openVpnAdiaFrame);
    QFormLayout *generalLayout = new QFormLayout(generalFrame);

    QWidget *customRenoInrWidget = new QWidget(this);
    initCheckWidget(m_customRenegotiaInrCheckbox, m_customRenoInrLabel, customRenoInrWidget);

    QWidget *customMtuWidget = new QWidget(this);
    initCheckWidget(m_customMtuCheckbox, m_customMtuLabel, customMtuWidget);

    QWidget *customUdpWidget = new QWidget(this);
    initCheckWidget(m_customUdpFragSizeCheckbox, m_customUdpLabel, customUdpWidget);

    QWidget *acceptAuthWidget = new QWidget(this);
    initCheckWidget(m_acceptAuthedPaksCheckbox, m_acceptAuthedPaksLabel, acceptAuthWidget);

    QWidget *exitRestarWidget = new QWidget(this);
    QHBoxLayout *exitRestarLayout = new QHBoxLayout(exitRestarWidget);
    exitRestarLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    exitRestarLayout->addWidget(m_specExitRestarCombox);
    exitRestarLayout->addWidget(m_specExRePingEdit);

    generalLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    generalLayout->addRow(m_customGatewayPortCheckbox, m_gatewayPortEdit);
    generalLayout->addRow(customRenoInrWidget, m_renogotiaInrEdit);
    generalLayout->addRow(m_compressionCheckbox, m_compressionCombox);
    generalLayout->addRow(m_tcpConnCheckbox);
    generalLayout->addRow(m_setVDevTypeCheckbox, m_setVDevTypeCombox);
    generalLayout->addRow(m_setVDevNameCheckbox, m_setVDevNameEdit);
    generalLayout->addRow(customMtuWidget, m_customMtuEdit);
    generalLayout->addRow(customUdpWidget, m_customUdpFragSizeEdit);
    generalLayout->addRow(m_tcpMssCheckbox);
    generalLayout->addRow(m_randomRemoteHostCheckbox);
    generalLayout->addRow(m_ipv6TunLinkCheckbox);
    generalLayout->addRow(m_specPingInrCheckbox, m_specPingInrEdit);
    generalLayout->addRow(m_specExitPingCheckbox, exitRestarWidget);
    generalLayout->addRow(acceptAuthWidget);
    generalLayout->addRow(m_specMaxRouteCheckbox, m_specMaxRouteEdit);

    //TLS settings
    QFrame *tlsSettingsFrame = new QFrame(m_openVpnAdiaFrame);
    QFormLayout *tlsLayout = new QFormLayout(tlsSettingsFrame);

    QWidget *usePreviousCertWidget = new QWidget(this);
    initCheckWidget(m_usePreviousCertCheckbox, usePreviousCertLabel, usePreviousCertWidget);

    QWidget *verifyPeerCertWidget = new QWidget(this);
    initCheckWidget(m_verifyPeerCertCheckbox, verifyPeerCertLabel, verifyPeerCertWidget);

    QWidget *keyfileWidget = new QWidget(this);
    QGridLayout *keyfileLayout = new QGridLayout(keyfileWidget);
    keyfileLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    keyfileLayout->addWidget(m_keyPathLabel, 0, 0);
    keyfileLayout->addWidget(m_keyPathEdit, 0, 1);
    keyfileLayout->addWidget(m_keyPathChooseBtn, 0, 2);
    keyfileLayout->addWidget(m_keyDirectionLabel, 1, 0);
    keyfileLayout->addWidget(m_keyDirectionCombox, 1, 1, 1, 2);
    m_keyDirectionLabel->setMinimumWidth(MIN_LABEL_WIDTH);

    tlsLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    tlsLayout->addRow(serverCertCheckLabel, m_serverCertCheckCombox);
    tlsLayout->addRow(m_subjectMatchLabel, m_subjectMatchEdit);
    tlsLayout->addRow(usePreviousCertWidget, m_usePreviousCertCombox);
    tlsLayout->addRow(verifyPeerCertWidget, m_verifyPeerCertCombox);
    tlsLayout->addRow(tlsModeLabel, m_tlsModeCombox);
    tlsLayout->addRow(keyfileWidget);

    //Proxies
    QFrame *proxiesFrame = new QFrame(m_openVpnAdiaFrame);
    QFormLayout *proxiesLayout = new QFormLayout(proxiesFrame);
    proxiesLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    proxiesLayout->addRow(proxyTypeLabel, m_proxyTypeCombox);
    proxiesLayout->addRow(m_proxyServerAddLabel, m_proxyServerAddEdit);
    proxiesLayout->addRow(m_proxyPortLabel, m_proxyPortEdit);
    proxiesLayout->addRow(nullptr, m_infiniteRetryCheckbox);
    proxiesLayout->addRow(m_proxyUsernameLabel, m_proxyUsernameEdit);
    proxiesLayout->addRow(m_proxyPwdLabel, m_proxyPwdEdit);
    m_proxyUsernameLabel->setMinimumWidth(MIN_LABEL_WIDTH);

    //Security
    QFrame *securityFrame = new QFrame(m_openVpnAdiaFrame);
    QFormLayout *securityLayout = new QFormLayout(securityFrame);
    securityLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    securityLayout->addRow(m_customKeySizeCheckbox, m_customKeySizeEdit);
    securityLayout->addRow(hmacLabel, m_hmacAuthCombox);

    //all
    openVpnAdiaVLayout->setSpacing(LAYOUT_SPACING_SMALL);
    openVpnAdiaVLayout->addWidget(generalLabel);
    openVpnAdiaVLayout->addWidget(generalFrame);
    openVpnAdiaVLayout->addSpacing(VLAYOUT_ADD_SPACING);
    openVpnAdiaVLayout->addWidget(tlsSettingLabel);
    openVpnAdiaVLayout->addWidget(tlsSettingsFrame);
    openVpnAdiaVLayout->addSpacing(VLAYOUT_ADD_SPACING);
    openVpnAdiaVLayout->addWidget(proxiesLabel);
    openVpnAdiaVLayout->addWidget(proxiesFrame);
    openVpnAdiaVLayout->addSpacing(VLAYOUT_ADD_SPACING);
    openVpnAdiaVLayout->addWidget(securityLabel);
    openVpnAdiaVLayout->addWidget(securityFrame);

    /**********控件显示**********/
    m_customGatewayPortCheckbox->setText(tr("Use custom gateway port")); //使用自定义网关端口
    m_compressionCheckbox->setText(tr("Use compression")); //使用压缩
    m_tcpConnCheckbox->setText(tr("Use a TCP connection")); //使用TCP连接
    m_setVDevTypeCheckbox->setText(tr("Set virtual device type")); //设置虚拟设备类型
    m_setVDevNameCheckbox->setText(tr("Set virtual device name")); //设置虚拟设备名称
    m_tcpMssCheckbox->setText(tr("Limit TCP Maximum Segment Size(MSS)")); //限制TCP最大段尺寸（MSS）
    m_randomRemoteHostCheckbox->setText(tr("Randomize remote hosts")); //随机化远程主机
    m_ipv6TunLinkCheckbox->setText(tr("IPv6 tun link")); //IPv6 tun连接
    m_specPingInrCheckbox->setText(tr("Specify ping interval")); //指定ping周期
    m_specExitPingCheckbox->setText(tr("Specify exit or restart ping")); //指定退出或重启的Ping
    m_specMaxRouteCheckbox->setText(tr("Specify max routes")); //指定路由上限
    m_infiniteRetryCheckbox->setText(tr("Infinite retry on error")); //出错时无限重试
    m_customKeySizeCheckbox->setText(tr("Use custom key size")); //使用自定义密钥大小
    m_keyPathChooseBtn->setText(tr("Choose")); //选择

    m_customRenoInrLabel->setFixedWidth(MAX_LABEL_WIDTH);
    m_customRenoInrLabel->setLabelText(tr("Use custom renegotiation interval")); //使用自定义重协商间隔
    m_customMtuLabel->setFixedWidth(MAX_LABEL_WIDTH);
    m_customMtuLabel->setLabelText(tr("Use custom tunnel Maximum Transmission Umit(MTU)")); //使用自定义隧道最大单元传输（MTU）
    m_customUdpLabel->setFixedWidth(MAX_LABEL_WIDTH);
    m_customUdpLabel->setLabelText(tr("Use custom UDP fragment size")); //使用自定义UDP分片大小
    m_acceptAuthedPaksLabel->setFixedWidth(440);//文本显示宽度
    m_acceptAuthedPaksLabel->setLabelText(tr("Accept authenticated packets from any address (Float)")); //接受来自任何地址（Float）已通过身份验证的数据包

    m_subjectMatchLabel->setText(tr("Subject Match")); //主题匹配
    m_keyPathLabel->setText(tr("Key File")); //密钥文件
    m_keyDirectionLabel->setText(tr("Key Direction")); //密钥方向

    m_proxyServerAddLabel->setText(tr("Server Address")); //服务器地址
    m_proxyPortLabel->setText(tr("Port")); //端口
    m_proxyUsernameLabel->setText(tr("Proxy USername")); //代理用户名
    m_proxyPwdLabel->setText(tr("Proxy Password")); //代理密码

    generalLabel->setText(tr("General")); //常规
    tlsSettingLabel->setText(tr("TLS settings"));
    serverCertCheckLabel->setText(tr("Server Certificate Check")); //服务器证书检验
    usePreviousCertLabel->setFixedWidth(MAX_LABEL_WIDTH);
    usePreviousCertLabel->setLabelText(tr("Use the previous authentication end (server) certificate")); //使用前面验证端（服务器）证书
    verifyPeerCertLabel->setFixedWidth(MAX_LABEL_WIDTH);
    verifyPeerCertLabel->setLabelText(tr("Verify peer (server) certificate nsCertType specification")); //验证对等点（服务器）证书nsCertType指定
    tlsModeLabel->setText(tr("Mode"));
    proxiesLabel->setText(tr("Proxies"));
    proxyTypeLabel->setText(tr("Proxy Type"));
    securityLabel->setText(tr("Security"));
    hmacLabel->setText(tr("HMAC Authentication")); //HMAC认证
    hmacLabel->setFixedWidth(268); //调整控件对齐=MAX_LABEL_WIDTH+28

    QFont font = generalLabel->font();
    font.setWeight(QFont::Weight::Medium);
    generalLabel->setFont(font);
    tlsSettingLabel->setFont(font);
    proxiesLabel->setFont(font);
    securityLabel->setFont(font);

    m_setVDevNameEdit->setPlaceholderText(tr("Input content")); //输入内容

    m_compressionCombox->addItem(tr("No"), KYCOMP2_LZODISABLE); //否
    m_compressionCombox->addItem("LZO", KYCOMP2_LZO);
    m_compressionCombox->addItem("LZ4", KYCOMP2_LZ4);
    m_compressionCombox->addItem("LZ4 v2", KYCOMP2_LZ4V2);
    m_compressionCombox->addItem(tr("Self-adaption"), KYCOMP2_LZOADAPTIVE); //自适应
    m_compressionCombox->addItem(tr("Automatic"), KYCOMP2_AUTO); //自动

    m_setVDevTypeCombox->addItem("TUN", KYVIRDEVTYPE_TUN);
    m_setVDevTypeCombox->addItem("TAP", KYVIRDEVTYPE_TAP);

    m_specExitRestarCombox->addItem(tr("Exit"), KYVPNPING_EXIT); //退出
    m_specExitRestarCombox->addItem(tr("Restart"), KYVPNPING_RESTART); //重启

    m_serverCertCheckCombox->addItem(tr("Don't verify certificate identification"), KYCHECKSERVER_NONE); //不验证证书标识
    m_serverCertCheckCombox->addItem(tr("Verify the entire subject exactly"), KYCHECKSERVER_ENTIRETHEME); //确切地验证整个主题
    m_serverCertCheckCombox->addItem(tr("Verify name exactly"), KYCHECKSERVER_ENTIRENAME); //精确验证名称
    m_serverCertCheckCombox->addItem(tr("Verify name by prefix"), KYCHECKSERVER_PRENAME); //按前缀验证名称

    m_usePreviousCertCombox->addItem(tr("Server"), KYVPNCERT_SERVER);
    m_usePreviousCertCombox->addItem(tr("Client"), KYVPNCERT_CLIENT);

    m_verifyPeerCertCombox->addItem(tr("Server"), KYVPNCERT_SERVER);
    m_verifyPeerCertCombox->addItem(tr("Client"), KYVPNCERT_CLIENT);

    m_tlsModeCombox->addItem(tr("None"), NONE); //无
    m_tlsModeCombox->addItem(tr("TLS-Certification"), TLS_CERT); //TLS-认证
    m_tlsModeCombox->addItem(tr("TLS-Encryption"), TLS_ENCRYPTION); //TLS-加密

    m_keyDirectionCombox->addItem(tr("None"), KEY_DIRECTION_NONE); //无
    m_keyDirectionCombox->addItem("0", KEY_DIRECTION_ZERO);
    m_keyDirectionCombox->addItem("1", KEY_DIRECTION_ONE);

    m_proxyTypeCombox->addItem(tr("Not Required"), NO); //不需要
    m_proxyTypeCombox->addItem("HTTP", HTTP);
    m_proxyTypeCombox->addItem("SOCKS", SOCKS);

    m_hmacAuthCombox->addItem(tr("Default"), KYHMACAUTH_DEFAULT); //默认
    m_hmacAuthCombox->addItem(tr("None"), KYHMACAUTH_NONE); //无
    m_hmacAuthCombox->addItem("MD-4", KYHMACAUTH_MD4);
    m_hmacAuthCombox->addItem("MD-5", KYHMACAUTH_MD5);
    m_hmacAuthCombox->addItem("SHA-1", KYHMACAUTH_SHA1);
    m_hmacAuthCombox->addItem("SHA-224", KYHMACAUTH_SHA224);
    m_hmacAuthCombox->addItem("SHA-256", KYHMACAUTH_SHA256);
    m_hmacAuthCombox->addItem("SHA-384", KYHMACAUTH_SHA384);
    m_hmacAuthCombox->addItem("SHA-512", KYHMACAUTH_SHA512);
    m_hmacAuthCombox->addItem("RIPEMD-160", KYHMACAUTH_RIPEMD160);

    //控件输入格式限制
    QRegExp rxNumber("^[0-9]+$");
    m_gatewayPortEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_renogotiaInrEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_customMtuEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_customUdpFragSizeEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_specPingInrEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_specExRePingEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_specMaxRouteEdit->setValidator(new QRegExpValidator(rxNumber, this));

    // IP的正则格式限制
    QRegExp rxIp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    m_proxyServerAddEdit->setValidator(new QRegExpValidator(rxIp, this));

    //default
    m_gatewayPortEdit->setText("1194");
    m_renogotiaInrEdit->setText("0");
    m_customMtuEdit->setText("1500");
    m_customUdpFragSizeEdit->setText("1500");
    m_specPingInrEdit->setText("30");
    m_specExRePingEdit->setText("30");
    m_specMaxRouteEdit->setText("30");
    if (m_serverCertCheckCombox->currentData().toInt() ==  (int)KYCHECKSERVER_NONE) {
        m_subjectMatchEdit->setEnabled(false);
    } else {
        m_subjectMatchEdit->setEnabled(true);
    }
    onTlsModeComboxIndexChanged();
    onProxyTypeComboxIndexChanged();
}

void VpnAdvancedPage::initSSwanAdiaFrame()
{
    /**********控件**********/
    QLabel *optionsLabel = new QLabel(this);
    QLabel *ikeLabel = new QLabel(this);
    QLabel *espLabel = new QLabel(this);
    m_optionsListWidget = new QListWidget(this);
    m_requestInIPCheckbox = new QCheckBox(this);
    m_udpEncapCheckbox = new QCheckBox(this);
    m_ipCompCheckbox = new QCheckBox(this);
    m_enablCustomCheckbox = new QCheckBox(this);
    m_ikeEdit = new QLineEdit(this);
    m_espEdit = new QLineEdit(this);
    m_ikeEspFrame = new QFrame(m_sSwanAdiaFrame);

    /**********布局**********/
    QVBoxLayout *sSwanAdiaVLayout = new QVBoxLayout(m_sSwanAdiaFrame);
    sSwanAdiaVLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    sSwanAdiaVLayout->setSpacing(LAYOUT_SPACING_SMALL);

    QFormLayout *ikeEspFrameLayout = new QFormLayout(m_ikeEspFrame);
    ikeEspFrameLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    ikeEspFrameLayout->setHorizontalSpacing(HOR_LAYOUT_SPACING);
    ikeEspFrameLayout->setVerticalSpacing(LAYOUT_ADD_SPACING);
    ikeEspFrameLayout->addRow(ikeLabel, m_ikeEdit);
    ikeEspFrameLayout->addRow(espLabel, m_espEdit);

    sSwanAdiaVLayout->addWidget(optionsLabel);
    sSwanAdiaVLayout->addWidget(m_optionsListWidget);
    sSwanAdiaVLayout->addSpacing(LAYOUT_ADD_SPACING);
    sSwanAdiaVLayout->addWidget(m_enablCustomCheckbox);
    sSwanAdiaVLayout->addWidget(m_ikeEspFrame);
    sSwanAdiaVLayout->addStretch();
    m_optionsListWidget->setFixedHeight(LISTWIDGET_HEIGHT_SMALL);

    /**********控件显示**********/
    optionsLabel->setText(tr("Options:")); //选项：
    ikeLabel->setText("IKE");
    espLabel->setText("ESP");
    m_requestInIPCheckbox->setText(tr("Request an inner IP address")); //请求内部IP地址
    m_udpEncapCheckbox->setText(tr("Enforce UDP encapsulation")); //强制UDP封装
    m_ipCompCheckbox->setText(tr("Use IP compression")); //使用IP压缩
    m_enablCustomCheckbox->setText(tr("Enable custom password suggestions")); //启用自定义密码建议

    m_optionsListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_optionsListWidget->setFrameShape(QFrame::Shape::StyledPanel);
    addListItem(m_optionsListWidget, m_requestInIPCheckbox);
    addListItem(m_optionsListWidget, m_udpEncapCheckbox);
    addListItem(m_optionsListWidget, m_ipCompCheckbox);

    //strong-Swan default
    m_enablCustomCheckbox->setChecked(false);
    m_ikeEspFrame->setEnabled(false);
}

void VpnAdvancedPage::initDefalutCheckState()
{
    m_mppeECheckbox->setChecked(false);
    m_mppeECombox->setEnabled(m_mppeECheckbox->isChecked());

    //openVPN
    m_customGatewayPortCheckbox->setChecked(true);
    m_customRenegotiaInrCheckbox->setChecked(true);
    m_compressionCheckbox->setChecked(true);
    m_tcpConnCheckbox->setChecked(true);
    m_setVDevTypeCheckbox->setChecked(true);
    m_setVDevNameCheckbox->setChecked(true);
    m_customMtuCheckbox->setChecked(true);
    m_customUdpFragSizeCheckbox->setChecked(true);
    m_tcpMssCheckbox->setChecked(true);
    m_randomRemoteHostCheckbox->setChecked(true);
    m_ipv6TunLinkCheckbox->setChecked(true);
    m_specPingInrCheckbox->setChecked(true);
    m_specExitPingCheckbox->setChecked(true);
    m_acceptAuthedPaksCheckbox->setChecked(true);
    m_specMaxRouteCheckbox->setChecked(true);

    m_gatewayPortEdit->setEnabled(m_customGatewayPortCheckbox->isChecked());
    m_renogotiaInrEdit->setEnabled(m_customRenegotiaInrCheckbox->isChecked());
    m_setVDevNameEdit->setEnabled(m_setVDevNameCheckbox->isChecked());
    m_customMtuEdit->setEnabled(m_customMtuCheckbox->isChecked());
    m_customUdpFragSizeEdit->setEnabled(m_customUdpFragSizeCheckbox->isChecked());
    m_specPingInrEdit->setEnabled(m_specPingInrCheckbox->isChecked());
    m_specExRePingEdit->setEnabled(m_specExitPingCheckbox->isChecked());
    m_specMaxRouteEdit->setEnabled(m_specMaxRouteCheckbox->isChecked());
    m_compressionCombox->setEnabled(m_compressionCheckbox->isChecked());
    m_setVDevTypeCombox->setEnabled(m_setVDevTypeCheckbox->isChecked());
    m_specExitRestarCombox->setEnabled(m_specExitPingCheckbox->isChecked());

    m_usePreviousCertCombox->setEnabled(m_usePreviousCertCheckbox->isChecked());
    m_verifyPeerCertCombox->setEnabled(m_verifyPeerCertCheckbox->isChecked());

    m_customKeySizeEdit->setEnabled(m_customKeySizeCheckbox->isChecked());
}

void VpnAdvancedPage::addListItem(QListWidget *listWidget, QWidget *widget)
{
    QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidget);
    QWidget *itemWidget = new QWidget(listWidget);
    QHBoxLayout *hLayout = new QHBoxLayout(itemWidget);
    hLayout->setContentsMargins(16, 0, 0, 0);
    hLayout->addWidget(widget);
    listWidgetItem->setSizeHint(QSize(listWidget->width(), ITEM_HEIGHT));
    listWidgetItem->setFlags(Qt::NoItemFlags);
    listWidget->addItem(listWidgetItem);
    listWidget->setItemWidget(listWidgetItem, itemWidget);
}

void VpnAdvancedPage::initCheckWidget(QCheckBox *checkBox, FixLabel *label, QWidget *widget)
{
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    hLayout->setSpacing(4);
    hLayout->addWidget(checkBox);
    hLayout->addWidget(label);
    hLayout->addStretch();
}

void VpnAdvancedPage::onProxyTypeComboxIndexChanged()
{
    switch (m_proxyTypeCombox->currentData().toInt()) {
    case NO:
        m_proxyServerAddEdit->clear();
        m_proxyPortEdit->clear();
        m_proxyUsernameEdit->clear();
        m_proxyPwdEdit->clear();
        m_infiniteRetryCheckbox->setChecked(false);

        m_proxyServerAddLabel->setEnabled(false);
        m_proxyPortLabel->setEnabled(false);
        m_proxyUsernameLabel->setEnabled(false);
        m_proxyPwdLabel->setEnabled(false);
        m_infiniteRetryCheckbox->setEnabled(false);
        m_proxyServerAddEdit->setEnabled(false);
        m_proxyPortEdit->setEnabled(false);
        m_proxyUsernameEdit->setEnabled(false);
        m_proxyPwdEdit->setEnabled(false);
        break;
    case HTTP:
        m_proxyServerAddLabel->setEnabled(true);
        m_proxyPortLabel->setEnabled(true);
        m_proxyUsernameLabel->setEnabled(true);
        m_proxyPwdLabel->setEnabled(true);
        m_infiniteRetryCheckbox->setEnabled(true);
        m_proxyServerAddEdit->setEnabled(true);
        m_proxyPortEdit->setEnabled(true);
        m_proxyUsernameEdit->setEnabled(true);
        m_proxyPwdEdit->setEnabled(true);
        break;
    case SOCKS:
        m_proxyServerAddLabel->setEnabled(true);
        m_proxyPortLabel->setEnabled(true);
        m_infiniteRetryCheckbox->setEnabled(true);
        m_proxyServerAddEdit->setEnabled(true);
        m_proxyPortEdit->setEnabled(true);
        m_proxyUsernameLabel->setEnabled(false);
        m_proxyPwdLabel->setEnabled(false);
        m_proxyUsernameEdit->setEnabled(false);
        m_proxyPwdEdit->setEnabled(false);
        break;
    default:
        break;
    }
}

void VpnAdvancedPage::onTlsModeComboxIndexChanged()
{
    switch (m_tlsModeCombox->currentData().toInt()) {
    case NONE:
        m_keyPathLabel->setEnabled(false);
        m_keyPathEdit->setEnabled(false);
        m_keyPathChooseBtn->setEnabled(false);
        m_keyDirectionLabel->setEnabled(false);
        m_keyDirectionCombox->setEnabled(false);
        break;
    case TLS_CERT:
        m_keyPathLabel->setEnabled(true);
        m_keyPathEdit->setEnabled(true);
        m_keyPathChooseBtn->setEnabled(true);
        m_keyDirectionLabel->setEnabled(true);
        m_keyDirectionCombox->setEnabled(true);
        break;
    case TLS_ENCRYPTION:
        m_keyPathLabel->setEnabled(true);
        m_keyPathEdit->setEnabled(true);
        m_keyPathChooseBtn->setEnabled(true);
        m_keyDirectionLabel->setEnabled(false);
        m_keyDirectionCombox->setEnabled(false);
        break;
    default:
        break;
    }
}

void VpnAdvancedPage::setEnableOfConfirmBtn()
{
    Q_EMIT setAdvancedPageState(checkConfirmBtnIsEnabled());
}

void VpnAdvancedPage::onKeyPathButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a private key"),
                                                    "recent:///",
                                                    tr("Key Files (*.key *.pem *.der *.p12 *.pfx)"));
    if (!fileName.isNull()) {
        m_keyPathEdit->blockSignals(true);
        m_keyPathEdit->setText(fileName);
        m_keyPathEdit->blockSignals(false);
    }
}

void VpnAdvancedPage::showL2tpAdiaPage()
{
    m_defaultAdiaFrame->setHidden(false);
    m_mruFrame->setHidden(false);
    m_compModeListWidget->item(COMP_MODE_PROTOCOL)->setHidden(false);
    m_compModeListWidget->item(COMP_MODE_ADDRESS)->setHidden(false);
    m_compModeListWidget->setFixedHeight(LISTWIDGET_HEIGHT);

    m_openVpnAdiaFrame->setHidden(true);
    m_sSwanAdiaFrame->setHidden(true);
}

void VpnAdvancedPage::showPptpAdiaPage()
{
    m_defaultAdiaFrame->setHidden(false);
    m_compModeListWidget->item(COMP_MODE_PROTOCOL)->setHidden(true);
    m_compModeListWidget->item(COMP_MODE_ADDRESS)->setHidden(true);
    m_compModeListWidget->setFixedHeight(LISTWIDGET_HEIGHT_SMALL);

    m_mruFrame->setHidden(true);
    m_openVpnAdiaFrame->setHidden(true);
    m_sSwanAdiaFrame->setHidden(true);
}

void VpnAdvancedPage::showOpenVpnAdiaPage()
{
    m_openVpnAdiaFrame->setHidden(false);

    m_defaultAdiaFrame->setHidden(true);
    m_mruFrame->setHidden(true);
    m_sSwanAdiaFrame->setHidden(true);
}

void VpnAdvancedPage::showSSwanAdiaPage()
{
    m_sSwanAdiaFrame->setHidden(false);

    m_defaultAdiaFrame->setHidden(true);
    m_mruFrame->setHidden(true);
    m_openVpnAdiaFrame->setHidden(true);
}

bool VpnAdvancedPage::checkConfirmBtnIsEnabled()
{
    //L2TP
    if (!m_mruFrame->isHidden()) {
        if (m_mruEdit->text().isEmpty() || m_mtuEdit->text().isEmpty()) {
            qDebug() << LOG_FLAG <<  "MRU or MTU is empty";
            return false;
        }
    }

    //PPTP 无必填项

    //OPEN VPN
    if (!m_openVpnAdiaFrame->isHidden()) {
        if (m_customGatewayPortCheckbox->isChecked() && m_gatewayPortEdit->text().isEmpty()) {
            return false;
        }

        if (m_customRenegotiaInrCheckbox->isChecked() && m_renogotiaInrEdit->text().isEmpty()) {
            return false;
        }

        if (m_setVDevNameCheckbox->isChecked() && m_setVDevNameEdit->text().isEmpty()) {
            return false;
        }

        if (m_customMtuCheckbox->isChecked() && m_customMtuEdit->text().isEmpty()) {
            return false;
        }

        if (m_customUdpFragSizeCheckbox->isChecked() && m_customUdpFragSizeEdit->text().isEmpty()) {
            return false;
        }

        if (m_specPingInrCheckbox->isChecked() && m_specPingInrEdit->text().isEmpty()) {
            return false;
        }

        if (m_specExitPingCheckbox->isChecked() && m_specExRePingEdit->text().isEmpty()) {
            return false;
        }

        if (m_specMaxRouteCheckbox->isChecked() && m_specMaxRouteEdit->text().isEmpty()) {
            return false;
        }

        if (m_serverCertCheckCombox->currentData().toInt() !=  (int)KYCHECKSERVER_NONE
                && m_subjectMatchEdit->text().isEmpty()) {
            return false;
        }

        if (m_tlsModeCombox->currentData().toInt() == TLS_CERT || m_tlsModeCombox->currentData().toInt() == TLS_ENCRYPTION) {
            if (m_keyPathEdit->text().isEmpty()) {
                return false;
            }
        }

        if (m_proxyTypeCombox->currentData().toInt() == HTTP || m_proxyTypeCombox->currentData().toInt() == SOCKS) {
            if (m_proxyServerAddEdit->text().isEmpty() || !getTextEditState(m_proxyServerAddEdit->text())) {
                return false;
            }
            if (m_proxyPortEdit->text().isEmpty()) {
                return false;
            }
        }

        if(m_customKeySizeCheckbox->isChecked() && m_customKeySizeEdit->text().isEmpty()) {
            return false;
        }
    }

    //strong-swan
    if (!m_sSwanAdiaFrame->isHidden()) {
        if (m_enablCustomCheckbox->isChecked()) {
            if (m_ikeEdit->text().isEmpty() || m_espEdit->text().isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

bool VpnAdvancedPage::getTextEditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}
