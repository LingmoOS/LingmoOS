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
#include "vpnconfigpage.h"
#include <QApplication>
#include <QFileDialog>

#define  NO_LAYOUT_MARGINS  0, 0, 0, 0
#define  NO_SPACE  0
#define  HLAYOUT_SPACING  16
#define  VLAYOUT_SPACING  24
#define  LEFT_LABEL_WIDTH  100
#define  PIN_MAX_LENGTH  10
#define  PWD_OPTION_THISUSER_INDEX 0
#define  PWD_OPTION_ALLUSERS_INDEX 1
#define  PWD_OPTION_ASKEVERYTIME_INDEX 2
#define  PWD_OPTION_NOTREQUIRED_INDEX 3
#define  LOG_FLAG  "[VPN ConfigPage]"

VpnConfigPage::VpnConfigPage(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
    onVpnTypeComboxIndexChanged();
}

VpnType VpnConfigPage::getVpnType()
{
    return VpnType(m_vpnTypeComboBox->currentData().toInt());
}

void VpnConfigPage::setVpnConfigInfo(const KyVpnConfig &vpnInfo)
{
    m_vpnNameEdit->setText(vpnInfo.m_vpnName);
    m_serverAddressEdit->setText(vpnInfo.m_gateway);
    m_vpnTypeComboBox->setCurrentIndex(vpnInfo.m_vpnType);
    onVpnTypeComboxIndexChanged();

    switch (vpnInfo.m_vpnType) {
    case KYVPNTYPE_L2TP:
        setL2tpConfigInfo(vpnInfo);
        break;
    case KYVPNTYPE_PPTP:
        setPptpConfigInfo(vpnInfo);
        break;
    case KYVPNTYPE_OPENVPN:
        setOpenVpnConfigInfo(vpnInfo);
        break;
    case KYVPNTYPE_STRONGSWAN:
        setSSwanVpnConfigInfo(vpnInfo);
        break;
    default:
        qDebug() << LOG_FLAG <<  "Unknow VPN Type" << vpnInfo.m_vpnType;
        break;
    }
}

bool VpnConfigPage::checkIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    int currentVpnType = m_vpnTypeComboBox->currentData().toInt();

    if (vpnInfo.m_vpnType != currentVpnType
            || vpnInfo.m_vpnName != m_vpnNameEdit->text()
            || vpnInfo.m_gateway != m_serverAddressEdit->text()) {

        isChanged = true;
    }

    switch (currentVpnType) {
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
    default:
        break;
    }

    return isChanged;
}

void VpnConfigPage::updateVpnConfigInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_vpnType = (KyVpnType)m_vpnTypeComboBox->currentData().toInt();
    vpnInfo.m_vpnName = m_vpnNameEdit->text();
    vpnInfo.m_gateway = m_serverAddressEdit->text();
    switch (m_vpnTypeComboBox->currentData().toInt()) {
    case L2TP:
        updateL2tpConfigInfo(vpnInfo);
        break;
    case OPEN_VPN:
        updateOpenVpnConfigInfo(vpnInfo);
        break;
    case PPTP:
        updatePptpConfigInfo(vpnInfo);
        break;
    case STRONG_SWAN:
        updateSSwanVpnConfigInfo(vpnInfo);
        break;
    default:
        break;
    }
}

void VpnConfigPage::initUI()
{
    /**********控件**********/
    //公有
    m_vpnTypeLabel = new QLabel(this);
    m_vpnNameLabel = new QLabel(this);
    m_serverAddressLabel = new FixLabel(this);
    m_vpnTypeComboBox = new QComboBox(this);
    m_vpnNameEdit = new QLineEdit(this);
    m_serverAddressEdit =  new QLineEdit(this);

    //认证方式 Authentication Mode
    m_authModeLabel = new FixLabel(this);
    m_authModeComboBox = new QComboBox(this);

    //证书 私钥
    m_caCertLabel = new FixLabel(this);
    m_userCertLabel = new FixLabel(this);
    m_privateKeyLabel = new FixLabel(this);
    m_privateKeyPwdLabel = new FixLabel(this);
    m_pwdOptionLabel = new FixLabel(this);

    m_caCertPathEdit = new QLineEdit(this);
    m_userCertPathEdit = new QLineEdit(this);
    m_privateKeyEdit = new QLineEdit(this);
    m_privateKeyPwdEdit = new KPasswordEdit(this);
    m_pwdOptionCombox = new QComboBox(this);

    m_caCertButton = new QPushButton(this);
    m_userCertButton = new QPushButton(this);
    m_privateKeyButton = new QPushButton(this);

    //静态密钥
    m_staticKeyLabel = new QLabel(this);
    m_keyDirectionLabel = new FixLabel(this);
    m_noticesLabel = new QLabel(this);
    m_localIpLabel = new QLabel(this);
    m_remoteIpLabel = new QLabel(this);

    m_staticKeyPathEdit = new QLineEdit(this);
    m_staticKeyButton = new QPushButton(this);
    m_keyDirectionCombox = new QComboBox(this);
    m_localIpEdit = new QLineEdit(this);
    m_remoteIpEdit = new QLineEdit(this);

    //智能卡 PIN码
    m_pinLabel = new QLabel(this);
    m_pinEdit = new QLineEdit(this);

    //密码
    m_usernameLabel = new FixLabel(this);
    m_userPwdLabel = new QLabel(this);
    m_userPwdOptionLabel = new FixLabel(this);
    m_usernameEdit = new QLineEdit(this);
    m_userPwdEdit = new KPasswordEdit(this);
    m_userPwdOptionCombox = new QComboBox(this);

    //L2TP PPTP公有
    m_ntDomainLabel = new QLabel(this);
    m_ntDomainEdit = new QLineEdit(this);


    /**********布局**********/
    m_vpnCommunalFrame = new QFrame(this);
    m_authModeFrame = new QFrame(this);
    m_caCertFrame = new QFrame(this);
    m_userCertFrame = new QFrame(this);
    m_staticKeysFrame = new QFrame(this);
    m_pinFrame = new QFrame(this);
    m_PrivateKeysFrame = new QFrame(this);
    m_usersFrame = new QFrame(this);
    m_userPwdOpFrame = new QFrame(this);
    m_ntFrame = new QFrame(this);

    //布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    mainLayout->setSpacing(VLAYOUT_SPACING);
    mainLayout->addWidget(m_vpnCommunalFrame);
    mainLayout->addWidget(m_authModeFrame);
    mainLayout->addWidget(m_caCertFrame);
    mainLayout->addWidget(m_userCertFrame);
    mainLayout->addWidget(m_staticKeysFrame);
    mainLayout->addWidget(m_pinFrame);
    mainLayout->addWidget(m_PrivateKeysFrame);
    mainLayout->addWidget(m_usersFrame);
    mainLayout->addWidget(m_userPwdOpFrame);
    mainLayout->addWidget(m_ntFrame);
    mainLayout->addSpacing(3);
    mainLayout->addStretch();

    QGridLayout *vpnCommLayout = new QGridLayout(m_vpnCommunalFrame);
    vpnCommLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    vpnCommLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    vpnCommLayout->setVerticalSpacing(VLAYOUT_SPACING);
    vpnCommLayout->addWidget(m_vpnTypeLabel, 0, 0);
    vpnCommLayout->addWidget(m_vpnNameLabel, 1, 0);
    vpnCommLayout->addWidget(m_serverAddressLabel, 2, 0);
    vpnCommLayout->addWidget(m_vpnTypeComboBox, 0, 1);
    vpnCommLayout->addWidget(m_vpnNameEdit, 1, 1);
    vpnCommLayout->addWidget(m_serverAddressEdit, 2, 1);
    m_vpnTypeLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_vpnNameLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_serverAddressLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *authMdLayout = new QHBoxLayout(m_authModeFrame);
    authMdLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    authMdLayout->setSpacing(HLAYOUT_SPACING);
    authMdLayout->addWidget(m_authModeLabel);
    authMdLayout->addWidget(m_authModeComboBox);
    m_authModeLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *caCertLayout = new QHBoxLayout(m_caCertFrame);
    caCertLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    caCertLayout->setSpacing(HLAYOUT_SPACING);
    caCertLayout->addWidget(m_caCertLabel);
    caCertLayout->addWidget(m_caCertPathEdit);
    caCertLayout->addWidget(m_caCertButton);
    m_caCertLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *userLayout = new QHBoxLayout(m_userCertFrame);
    userLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    userLayout->setSpacing(HLAYOUT_SPACING);
    userLayout->addWidget(m_userCertLabel);
    userLayout->addWidget(m_userCertPathEdit);
    userLayout->addWidget(m_userCertButton);
    m_userCertLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QGridLayout *staticKeyLayout = new QGridLayout(m_staticKeysFrame);
    staticKeyLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    staticKeyLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    staticKeyLayout->setVerticalSpacing(VLAYOUT_SPACING);
    staticKeyLayout->addWidget(m_staticKeyLabel, 0, 0);
    staticKeyLayout->addWidget(m_staticKeyPathEdit, 0, 1);
    staticKeyLayout->addWidget(m_staticKeyButton, 0, 2);
    staticKeyLayout->addWidget(m_keyDirectionLabel, 1, 0);
    staticKeyLayout->addWidget(m_keyDirectionCombox, 1, 1, 1, 2);
    staticKeyLayout->addWidget(m_noticesLabel, 2, 0, 2, 3);
    staticKeyLayout->addWidget(m_localIpLabel, 4, 0);
    staticKeyLayout->addWidget(m_localIpEdit, 4, 1, 1, 2);
    staticKeyLayout->addWidget(m_remoteIpLabel, 5, 0);
    staticKeyLayout->addWidget(m_remoteIpEdit, 5, 1, 1, 2);
    m_staticKeyLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_keyDirectionLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_localIpLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_remoteIpLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *pinLayout = new QHBoxLayout(m_pinFrame);
    pinLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    pinLayout->setSpacing(HLAYOUT_SPACING);
    pinLayout->addWidget(m_pinLabel);
    pinLayout->addWidget(m_pinEdit);
    m_pinLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QGridLayout *privateKeysLayout = new QGridLayout(m_PrivateKeysFrame);
    privateKeysLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    privateKeysLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    privateKeysLayout->setVerticalSpacing(VLAYOUT_SPACING);
    privateKeysLayout->addWidget(m_privateKeyLabel, 0, 0);
    privateKeysLayout->addWidget(m_privateKeyPwdLabel, 1, 0);
    privateKeysLayout->addWidget(m_pwdOptionLabel, 2, 0);
    privateKeysLayout->addWidget(m_privateKeyEdit, 0, 1);
    privateKeysLayout->addWidget(m_privateKeyButton, 0, 2);
    privateKeysLayout->addWidget(m_privateKeyPwdEdit, 1, 1, 1, 2);
    privateKeysLayout->addWidget(m_pwdOptionCombox, 2, 1, 1, 2);
    m_privateKeyLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_privateKeyPwdLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_pwdOptionLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QGridLayout *usersLayout = new QGridLayout(m_usersFrame);
    usersLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    usersLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    usersLayout->setVerticalSpacing(VLAYOUT_SPACING);
    usersLayout->addWidget(m_usernameLabel, 0, 0);
    usersLayout->addWidget(m_usernameEdit, 0, 1);
    usersLayout->addWidget(m_userPwdLabel, 1, 0);
    usersLayout->addWidget(m_userPwdEdit, 1, 1);
    m_usernameLabel->setFixedWidth(LEFT_LABEL_WIDTH);
    m_userPwdLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *userPwdOLayout = new QHBoxLayout(m_userPwdOpFrame);
    userPwdOLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    userPwdOLayout->setSpacing(HLAYOUT_SPACING);
    userPwdOLayout->addWidget(m_userPwdOptionLabel);
    userPwdOLayout->addWidget(m_userPwdOptionCombox);
    m_userPwdOptionLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    QHBoxLayout *ntLayout = new QHBoxLayout(m_ntFrame);
    ntLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    ntLayout->setSpacing(HLAYOUT_SPACING);
    ntLayout->addWidget(m_ntDomainLabel);
    ntLayout->addWidget(m_ntDomainEdit);
    m_ntDomainLabel->setFixedWidth(LEFT_LABEL_WIDTH);

    /**********控件显示**********/
    m_vpnTypeLabel->setText(tr("Type")); //类型
    m_vpnNameLabel->setText(tr("Name")); //名称
    m_staticKeyLabel->setText(tr("Static Key")); //静态密钥
    m_localIpLabel->setText(tr("Local IP")); //本地IP地址
    m_remoteIpLabel->setText(tr("Remote IP")); //远程IP地址
    m_pinLabel->setText(tr("PIN Code")); //PIN码
    m_userPwdLabel->setText(tr("Password")); //密码
    m_ntDomainLabel->setText(tr("NT Domain")); //NT域

    m_serverAddressLabel->setLabelText(tr("Server Address")); //服务器地址
    m_authModeLabel->setLabelText(tr("Authentication Mode")); //认证方式
    m_caCertLabel->setLabelText(tr("CA Certificate")); //CA证书
    m_userCertLabel->setLabelText(tr("User Certificate")); //用户证书
    m_keyDirectionLabel->setLabelText(tr("Key Direction")); //密钥方向
    m_privateKeyLabel->setLabelText(tr("Private Key")); //私钥
    m_privateKeyPwdLabel->setLabelText(tr("Private Key Password")); //私有密钥密码
    m_pwdOptionLabel->setLabelText(tr("Password Options")); //密码选项
    m_usernameLabel->setLabelText(tr("Username")); //用户名
    m_userPwdOptionLabel->setLabelText(tr("Password Options")); //密码选项

    m_noticesLabel->setWordWrap(true);
    m_noticesLabel->setText(tr("Notice:\nIf key direction is used, it must be opposite to the VPN side used. "
                               "If '1' is used, the connection must use '0'. "
                               "If you are not sure which value to use, please contact your system administrator."));
    QPalette pal;
    pal.setColor(QPalette::WindowText, qApp->palette().color(QPalette::Mid));
    m_noticesLabel->setPalette(pal);

    m_caCertButton->setText(tr("Choose")); //选择
    m_userCertButton->setText(tr("Choose"));
    m_staticKeyButton->setText(tr("Choose"));
    m_privateKeyButton->setText(tr("Choose"));

    m_vpnTypeComboBox->addItem("L2TP", L2TP);
    m_vpnTypeComboBox->addItem("open VPN", OPEN_VPN);
    m_vpnTypeComboBox->addItem("PPTP", PPTP);
    m_vpnTypeComboBox->addItem("strong-swan", STRONG_SWAN);

    m_keyDirectionCombox->addItem(tr("None"), KEY_DIRECTION_NONE);
    m_keyDirectionCombox->addItem("0", KEY_DIRECTION_ZERO);
    m_keyDirectionCombox->addItem("1", KEY_DIRECTION_ONE);

    m_pwdOptionCombox->addItem(tr("Save password only for this user"), KYPASSWD_FORTHISUSER); //仅对当前用户保存密码
    m_pwdOptionCombox->addItem(tr("Save password for all users"), KYPASSWD_FORALLUSER); //为所有用户保存密码
    m_pwdOptionCombox->addItem(tr("Ask password every time"), KYPASSWD_ASKEVERYTIME); //每次都询问
    m_pwdOptionCombox->addItem(tr("Don't require a password"), KYPASSWD_ISNOTREQUIRED);//不需要密码
    m_pwdOptionCombox->setCurrentIndex(PWD_OPTION_ASKEVERYTIME_INDEX);

    m_userPwdOptionCombox->addItem(tr("Save password only for this user"), KYPASSWD_FORTHISUSER);
    m_userPwdOptionCombox->addItem(tr("Save password for all users"), KYPASSWD_FORALLUSER);
    m_userPwdOptionCombox->addItem(tr("Ask password every time"), KYPASSWD_ASKEVERYTIME);
    m_userPwdOptionCombox->addItem(tr("Don't require a password"), KYPASSWD_ISNOTREQUIRED);
    m_userPwdOptionCombox->setCurrentIndex(PWD_OPTION_ASKEVERYTIME_INDEX);

    m_vpnNameEdit->setPlaceholderText(tr("Required")); //必填
    m_serverAddressEdit->setPlaceholderText(tr("Required"));
    m_privateKeyPwdEdit->setClearButtonEnabled(false);
    m_userPwdEdit->setClearButtonEnabled(false);

    //控件输入格式限制
    QRegExp rxNumber("^[0-9]+$");
    m_pinEdit->setValidator(new QRegExpValidator(rxNumber, this));
    m_pinEdit->setMaxLength(PIN_MAX_LENGTH);
    // IP的正则格式限制
    QRegExp rxIp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    QRegExpValidator *ipExpVal = new QRegExpValidator(rxIp, this);
    m_serverAddressEdit->setValidator(ipExpVal);
    m_localIpEdit->setValidator(ipExpVal);
    m_remoteIpEdit->setValidator(ipExpVal);
}

void VpnConfigPage::initConnect()
{
    connect(m_vpnTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VpnConfigPage::onVpnTypeComboxIndexChanged);
    connect(m_authModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VpnConfigPage::onAuthModeComboxIndexChanged);

    connect(m_vpnNameEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_serverAddressEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_caCertPathEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_userCertPathEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_privateKeyEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_staticKeyPathEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_localIpEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_remoteIpEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_pinEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_ntDomainEdit, &QLineEdit::textChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);

    connect(m_vpnTypeComboBox, &QComboBox::currentTextChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_authModeComboBox, &QComboBox::currentTextChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_pwdOptionCombox, &QComboBox::currentTextChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);
    connect(m_userPwdOptionCombox, &QComboBox::currentTextChanged, this, &VpnConfigPage::setEnableOfConfirmBtn);

    connect(m_caCertButton, &QPushButton::clicked, this, &VpnConfigPage::onCaCertButtonClicked);
    connect(m_userCertButton, &QPushButton::clicked, this, &VpnConfigPage::onUserCertButtonClicked);
    connect(m_staticKeyButton, &QPushButton::clicked, this, &VpnConfigPage::onStaticKeyButtonClicked);
    connect(m_privateKeyButton, &QPushButton::clicked, this, &VpnConfigPage::onPrivateKeyButtonClicked);
}

void VpnConfigPage::showL2tpPwd()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_usersFrame->setHidden(false);
    m_userPwdOpFrame->setHidden(false);
    m_ntFrame->setHidden(false);

    m_PrivateKeysFrame->setHidden(true);
    m_caCertFrame->setHidden(true);
    m_userCertFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
}

void VpnConfigPage::showPptpPwd()
{
    m_vpnCommunalFrame->setHidden(false);
    m_usersFrame->setHidden(false);
    m_userPwdOpFrame->setHidden(false);
    m_ntFrame->setHidden(false);

    m_authModeFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_caCertFrame->setHidden(true);
    m_userCertFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
}

void VpnConfigPage::showOpenVpnTls()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_PrivateKeysFrame->setHidden(false);
    m_caCertFrame->setHidden(false);
    m_userCertFrame->setHidden(false);

    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_usersFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showOpenVpnPwd()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_caCertFrame->setHidden(false);
    m_usersFrame->setHidden(false);
    m_userPwdOpFrame->setHidden(false);

    m_userCertFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showOpenVpnPwdTls()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_caCertFrame->setHidden(false);
    m_userCertFrame->setHidden(false);
    m_PrivateKeysFrame->setHidden(false);
    m_usersFrame->setHidden(false);
    m_userPwdOpFrame->setHidden(false);

    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showOpenVpnStaticKey()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_staticKeysFrame->setHidden(false);

    m_caCertFrame->setHidden(true);
    m_userCertFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_usersFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showSswanCertPrivteKey()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_userCertFrame->setHidden(false);
    m_PrivateKeysFrame->setHidden(false);

    m_caCertFrame->setHidden(true);
    m_usersFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showSswanCertSsh()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_userCertFrame->setHidden(false);

    m_caCertFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_usersFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showSswanSmartCard()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_pinFrame->setHidden(false);

    m_caCertFrame->setHidden(true);
    m_userCertFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_usersFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

void VpnConfigPage::showSswanEap()
{
    m_vpnCommunalFrame->setHidden(false);
    m_authModeFrame->setHidden(false);
    m_usersFrame->setHidden(false);

    m_caCertFrame->setHidden(true);
    m_userCertFrame->setHidden(true);
    m_PrivateKeysFrame->setHidden(true);
    m_userPwdOpFrame->setHidden(true);
    m_staticKeysFrame->setHidden(true);
    m_pinFrame->setHidden(true);
    m_ntFrame->setHidden(true);
}

bool VpnConfigPage::getTextEditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}

bool VpnConfigPage::checkConfirmBtnIsEnabled()
{
    if (m_vpnNameEdit->text().isEmpty()) {
        qDebug() << LOG_FLAG <<  "VPN name empty or invalid";
        return false;
    }

    if (m_serverAddressEdit->text().isEmpty() || !getTextEditState(m_serverAddressEdit->text())) {
        qDebug() << LOG_FLAG <<  "VPN server address empty or invalid";
        return false;
    }

    int currentAuthMethod = m_authModeComboBox->currentData().toInt();
    switch (currentAuthMethod) {
    case KYAUTH_PASSWD:
    case KYAUTH_EAP:
        if (m_usernameEdit->text().isEmpty() && !m_userPwdEdit->text().isEmpty()) {
            return false;
        }
        if (m_vpnTypeComboBox->currentData().toInt() == OPEN_VPN && m_caCertPathEdit->text().isEmpty()) {
            return false;
        }
        break;

    case KYAUTH_CERTIFICATE:
        if (m_caCertPathEdit->text().isEmpty() || m_userCertPathEdit->text().isEmpty()
                || m_privateKeyEdit->text().isEmpty()) {
            return false;
        }
        break;

    case KYAUTH_CERTIFICATEANDPASSWD:
        if (m_caCertPathEdit->text().isEmpty() || m_userCertPathEdit->text().isEmpty()
                || m_privateKeyEdit->text().isEmpty()) {
            return false;
        }
        if (m_usernameEdit->text().isEmpty() && !m_userPwdEdit->text().isEmpty()) {
            return false;
        }
        break;

    case KYAUTH_STATICPASSWD:
        if (m_staticKeyPathEdit->text().isEmpty()) {
            return false;
        }
        break;

    default:
        break;
    }

    return true;
}

void VpnConfigPage::setPwdAuthInfo(const QString &username, const QString &userPwd, const KyPasswdPolicy &userPwdPolicy)
{
    m_usernameEdit->setText(username);
    m_userPwdEdit->setText(userPwd);
    if (userPwdPolicy == KYPASSWD_ISNOTREQUIRED) {
        m_userPwdOptionCombox->setCurrentIndex(PWD_OPTION_NOTREQUIRED_INDEX);
    } else {
        m_userPwdOptionCombox->setCurrentIndex(userPwdPolicy);
    }
}

void VpnConfigPage::setCaCertAuthInfo(const QString &caCertificate)
{
    m_caCertPathEdit->setText(caCertificate);
}

void VpnConfigPage::setKeyAuthInfo(const QString &userCert, const QString &privateKey,
                                   const QString &privateKeyPwd, const KyPasswdPolicy &privatePwdPolicy)
{
    m_userCertPathEdit->setText(userCert);
    m_privateKeyEdit->setText(privateKey);
    m_privateKeyPwdEdit->setText(privateKeyPwd);
    if (privatePwdPolicy == KYPASSWD_ISNOTREQUIRED) {
        m_userPwdOptionCombox->setCurrentIndex(PWD_OPTION_NOTREQUIRED_INDEX);
    } else {
        m_userPwdOptionCombox->setCurrentIndex(privatePwdPolicy);
    }
}

void VpnConfigPage::setStaticKeyAuthInfo(const QString &staticKey, const QString &vpnKeyDirection,
                                         const QString &localIp, const QString &remoteIp)
{
    m_staticKeyPathEdit->setText(staticKey);
    m_localIpEdit->setText(localIp);
    m_remoteIpEdit->setText(remoteIp);
    if (vpnKeyDirection == KEY_DIRECTION_ZERO) {
        m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_ZERO_INDEX);
    } else if (vpnKeyDirection == KEY_DIRECTION_ONE) {
        m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_ONE_INDEX);
    } else {
        m_keyDirectionCombox->setCurrentIndex(KEY_DIRECTION_NONE_INDEX);
    }
}

void VpnConfigPage::setAgentAuthInfo(const QString &userCert)
{
    m_userCertPathEdit->setText(userCert);
}

void VpnConfigPage::setSmartCardAuthInfo(const QString &pin)
{
    m_pinEdit->setText(pin);
}

void VpnConfigPage::setEapAuthInfo(const QString &username, const QString &userPwd)
{
    m_usernameEdit->setText(username);
    m_userPwdEdit->setText(userPwd);
}

void VpnConfigPage::setNtDomain(const QString &ntDomain)
{
    m_ntDomainEdit->setText(ntDomain);
}

void VpnConfigPage::setL2tpConfigInfo(const KyVpnConfig &vpnInfo)
{
    if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_PASSWD) {
        m_authModeComboBox->setCurrentIndex(KYAUTH_PASSWD);
        setPwdAuthInfo(vpnInfo.m_userName, vpnInfo.m_userPasswd, vpnInfo.m_passwdPolicy);
        setNtDomain(vpnInfo.m_ntDomain);
    }
}

void VpnConfigPage::setPptpConfigInfo(const KyVpnConfig &vpnInfo)
{
    m_authModeComboBox->clear();
    setPwdAuthInfo(vpnInfo.m_userName, vpnInfo.m_userPasswd, vpnInfo.m_passwdPolicy);
    setNtDomain(vpnInfo.m_ntDomain);
}

void VpnConfigPage::setOpenVpnConfigInfo(const KyVpnConfig &vpnInfo)
{
    if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_PASSWD) {
        m_authModeComboBox->setCurrentIndex(PASSWD_INDEX);
        setCaCertAuthInfo(vpnInfo.m_caCertificate);
        setPwdAuthInfo(vpnInfo.m_userName, vpnInfo.m_userPasswd, vpnInfo.m_passwdPolicy);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_CERTIFICATE) {
        m_authModeComboBox->setCurrentIndex(CERTIFICATE_INDEX);
        setCaCertAuthInfo(vpnInfo.m_caCertificate);
        setKeyAuthInfo(vpnInfo.m_userCertificate, vpnInfo.m_userKey,
                       vpnInfo.m_privatePasswd, vpnInfo.m_privatePasswdPolicy);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_STATICPASSWD) {
        m_authModeComboBox->setCurrentIndex(STATICPASSWD_INDEX);
        setStaticKeyAuthInfo(vpnInfo.m_staticKey, vpnInfo.m_vpnKeyDir,
                             vpnInfo.m_localAddress, vpnInfo.m_remoteAddress);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_CERTIFICATEANDPASSWD) {
        m_authModeComboBox->setCurrentIndex(CERTIFICATEANDPASSWD_INDEX);
        setCaCertAuthInfo(vpnInfo.m_caCertificate);
        setKeyAuthInfo(vpnInfo.m_userCertificate, vpnInfo.m_userKey,
                       vpnInfo.m_privatePasswd, vpnInfo.m_privatePasswdPolicy);
        setPwdAuthInfo(vpnInfo.m_userName, vpnInfo.m_userPasswd, vpnInfo.m_passwdPolicy);
    }
}

void VpnConfigPage::setSSwanVpnConfigInfo(const KyVpnConfig &vpnInfo)
{
    if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_KEY) {
        m_authModeComboBox->setCurrentIndex(KEY_INDEX);
        setCaCertAuthInfo(vpnInfo.m_caCertificate);
        setKeyAuthInfo(vpnInfo.m_userCertificate, vpnInfo.m_userKey,
                       vpnInfo.m_privatePasswd, vpnInfo.m_privatePasswdPolicy);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_AGENT) {
        m_authModeComboBox->setCurrentIndex(AGENT_INDEX);
        setAgentAuthInfo(vpnInfo.m_userCertificate);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_SMARTCARD) {
        m_authModeComboBox->setCurrentIndex(SMARTCARD_INDEX);
        setSmartCardAuthInfo(vpnInfo.m_pinId);

    } else if (vpnInfo.m_authMethod == KyAuthMethod::KYAUTH_EAP) {
        m_authModeComboBox->setCurrentIndex(EAP_INDEX);
        setEapAuthInfo(vpnInfo.m_userName, vpnInfo.m_userPasswd);
    }
}

void VpnConfigPage::updateL2tpConfigInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_authMethod = (KyAuthMethod)m_authModeComboBox->currentData().toInt();
    vpnInfo.m_ntDomain = m_ntDomainEdit->text();
    vpnInfo.m_userName = m_usernameEdit->text();
    vpnInfo.m_userPasswd = m_userPwdEdit->text();
    vpnInfo.m_passwdPolicy = (KyPasswdPolicy)m_userPwdOptionCombox->currentData().toInt();
}

void VpnConfigPage::updatePptpConfigInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_ntDomain = m_ntDomainEdit->text();
    vpnInfo.m_userName = m_usernameEdit->text();
    vpnInfo.m_userPasswd = m_userPwdEdit->text();
    vpnInfo.m_passwdPolicy = (KyPasswdPolicy)m_userPwdOptionCombox->currentData().toInt();
}

void VpnConfigPage::updateOpenVpnConfigInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_authMethod = (KyAuthMethod)m_authModeComboBox->currentData().toInt();
    if (vpnInfo.m_authMethod == KYAUTH_CERTIFICATE || vpnInfo.m_authMethod == KYAUTH_CERTIFICATEANDPASSWD) {
        vpnInfo.m_caCertificate = m_caCertPathEdit->text();
        vpnInfo.m_userCertificate = m_userCertPathEdit->text();
        vpnInfo.m_userKey = m_privateKeyEdit->text();
        vpnInfo.m_privatePasswd = m_privateKeyPwdEdit->text();
        vpnInfo.m_privatePasswdPolicy = (KyPasswdPolicy)m_pwdOptionCombox->currentData().toInt();
    }

    if (vpnInfo.m_authMethod == KYAUTH_CERTIFICATEANDPASSWD || vpnInfo.m_authMethod == KYAUTH_PASSWD) {
        vpnInfo.m_userName = m_usernameEdit->text();
        vpnInfo.m_userPasswd = m_userPwdEdit->text();
        vpnInfo.m_passwdPolicy = (KyPasswdPolicy)m_userPwdOptionCombox->currentData().toInt();
        vpnInfo.m_caCertificate = m_caCertPathEdit->text();
    }

    if (vpnInfo.m_authMethod == KYAUTH_STATICPASSWD) {
        vpnInfo.m_staticKey = m_staticKeyPathEdit->text();
        vpnInfo.m_vpnKeyDir = m_keyDirectionCombox->currentData().toString();
        vpnInfo.m_localAddress = m_localIpEdit->text();
        vpnInfo.m_remoteAddress = m_remoteIpEdit->text();
    }
}

void VpnConfigPage::updateSSwanVpnConfigInfo(KyVpnConfig &vpnInfo)
{
    vpnInfo.m_authMethod = (KyAuthMethod)m_authModeComboBox->currentData().toInt();
    if (vpnInfo.m_authMethod == KYAUTH_KEY) {
        vpnInfo.m_caCertificate = m_caCertPathEdit->text();
        vpnInfo.m_userCertificate = m_userCertPathEdit->text();
        vpnInfo.m_userKey = m_privateKeyEdit->text();
        vpnInfo.m_privatePasswd = m_privateKeyPwdEdit->text();
        vpnInfo.m_privatePasswdPolicy = (KyPasswdPolicy)m_pwdOptionCombox->currentData().toInt();

    } else if (vpnInfo.m_authMethod == KYAUTH_AGENT) {
        vpnInfo.m_userCertificate = m_userCertPathEdit->text();

    } else if (vpnInfo.m_authMethod == KYAUTH_SMARTCARD) {
        vpnInfo.m_pinId = m_pinEdit->text();

    } else if (vpnInfo.m_authMethod == KYAUTH_EAP) {
        vpnInfo.m_userName = m_usernameEdit->text();
        vpnInfo.m_userPasswd = m_userPwdEdit->text();
    }
}

void VpnConfigPage::onVpnTypeComboxIndexChanged()
{
    switch (m_vpnTypeComboBox->currentData().toInt()) {
    case L2TP:
        m_authModeComboBox->clear();
        m_authModeComboBox->addItem(tr("Password"), KYAUTH_PASSWD);
        onAuthModeComboxIndexChanged();
        break;
    case OPEN_VPN:
        m_authModeComboBox->clear();
        m_authModeComboBox->addItem(tr("Certificate(TLS)"), KYAUTH_CERTIFICATE);
        m_authModeComboBox->addItem(tr("Static key"), KYAUTH_STATICPASSWD);
        m_authModeComboBox->addItem(tr("Password"), KYAUTH_PASSWD);
        m_authModeComboBox->addItem(tr("Password and certificate(TLS)"), KYAUTH_CERTIFICATEANDPASSWD);
        onAuthModeComboxIndexChanged();
        break;
    case PPTP:
        m_authModeComboBox->clear();
        showPptpPwd();
        break;
    case STRONG_SWAN:
        m_authModeComboBox->clear();
        m_authModeComboBox->addItem(tr("Certificate/Private key"), KYAUTH_KEY);
        m_authModeComboBox->addItem(tr("Certificate/ssh-agent"), KYAUTH_AGENT);
        m_authModeComboBox->addItem(tr("Smart card"), KYAUTH_SMARTCARD);
        m_authModeComboBox->addItem("EAP", KYAUTH_EAP);
        onAuthModeComboxIndexChanged();
        break;
    default:
        break;
    }
    Q_EMIT vpnTypeChanged(VpnType(m_vpnTypeComboBox->currentData().toInt()));
}

void VpnConfigPage::onAuthModeComboxIndexChanged()
{
    switch (m_authModeComboBox->currentData().toInt()) {
    case KYAUTH_PASSWD:
        if (m_vpnTypeComboBox->currentData().toInt() == L2TP) {
            showL2tpPwd();
        } else if (m_vpnTypeComboBox->currentData().toInt() == OPEN_VPN) {
            showOpenVpnPwd();
        }
        break;
    case KYAUTH_KEY:
        showSswanCertPrivteKey();
        break;
    case KYAUTH_AGENT:
        showSswanCertSsh();
        break;
    case KYAUTH_SMARTCARD:
        showSswanSmartCard();
        break;
    case KYAUTH_EAP:
        showSswanEap();
        break;
    case KYAUTH_CERTIFICATE:
        showOpenVpnTls();
        break;
    case KYAUTH_CERTIFICATEANDPASSWD:
        showOpenVpnPwdTls();
        break;
    case KYAUTH_STATICPASSWD:
        showOpenVpnStaticKey();
        break;
    default:
        qDebug() << LOG_FLAG <<  "Unknow VPN auth mothod";
        break;
    }
}

void VpnConfigPage::setEnableOfConfirmBtn()
{
    Q_EMIT setConfigPageState(checkConfirmBtnIsEnabled());
}

void VpnConfigPage::onPrivateKeyButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a private key"),
                                                    "recent:///",
                                                    tr("Key Files (*.key *.pem *.der *.p12 *.pfx)"));
    if (!fileName.isNull()) {
        m_privateKeyEdit->blockSignals(true);
        m_privateKeyEdit->setText(fileName);
        m_privateKeyEdit->blockSignals(false);
    }
}

void VpnConfigPage::onCaCertButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a CA certificate"),
                                                    "recent:///",
                                                    tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
    if (!fileName.isNull()) {
        m_caCertPathEdit->blockSignals(true);
        m_caCertPathEdit->setText(fileName);
        m_caCertPathEdit->blockSignals(false);
    }
}

void VpnConfigPage::onUserCertButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a User certificate"),
                                                    "recent:///",
                                                    tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
    if (!fileName.isNull()) {
        m_userCertPathEdit->blockSignals(true);
        m_userCertPathEdit->setText(fileName);
        m_userCertPathEdit->blockSignals(false);
    }
}

void VpnConfigPage::onStaticKeyButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a Static key"),
                                                    "recent:///",
                                                    tr("Key Files (*.key *.pem *.der *.p12 *.pfx)"));
    if (!fileName.isNull()) {
        m_staticKeyPathEdit->blockSignals(true);
        m_staticKeyPathEdit->setText(fileName);
        m_staticKeyPathEdit->blockSignals(false);
    }
}


bool VpnConfigPage::checkL2tpIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    if (vpnInfo.m_ntDomain != m_ntDomainEdit->text()
            || vpnInfo.m_userName != m_usernameEdit->text()
            || vpnInfo.m_userPasswd != m_userPwdEdit->text()
            || vpnInfo.m_passwdPolicy != m_userPwdOptionCombox->currentData().toInt()) {

        isChanged = true;
    }
    return isChanged;
}

bool VpnConfigPage::checkPptpIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    if (vpnInfo.m_ntDomain != m_ntDomainEdit->text()
            || vpnInfo.m_userName != m_usernameEdit->text()
            || vpnInfo.m_userPasswd != m_userPwdEdit->text()
            || vpnInfo.m_passwdPolicy != m_userPwdOptionCombox->currentData().toInt()) {

        isChanged = true;
    }
    return isChanged;
}

bool VpnConfigPage::checkOpenVpnIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    int currentAuthMethod = m_authModeComboBox->currentData().toInt();
    if (vpnInfo.m_authMethod != currentAuthMethod) {
        isChanged = true;
    }

    if (currentAuthMethod == KYAUTH_PASSWD || currentAuthMethod == KYAUTH_CERTIFICATEANDPASSWD) {
        if (vpnInfo.m_userName != m_usernameEdit->text()
                || vpnInfo.m_userPasswd != m_userPwdEdit->text()
                || vpnInfo.m_passwdPolicy != m_userPwdOptionCombox->currentData().toInt()
                || vpnInfo.m_caCertificate != m_caCertPathEdit->text()) {

            isChanged = true;
        }
    }

    if (currentAuthMethod == KYAUTH_CERTIFICATEANDPASSWD || currentAuthMethod == KYAUTH_CERTIFICATE) {
        if (vpnInfo.m_caCertificate != m_caCertPathEdit->text()
                || vpnInfo.m_userCertificate != m_userCertPathEdit->text()
                || vpnInfo.m_userKey != m_privateKeyEdit->text()
                || vpnInfo.m_privatePasswd != m_privateKeyPwdEdit->text()
                || vpnInfo.m_privatePasswdPolicy != m_pwdOptionCombox->currentData().toInt()) {

            isChanged = true;
        }

    }
    if (currentAuthMethod == KYAUTH_STATICPASSWD) {
        if (vpnInfo.m_staticKey != m_staticKeyPathEdit->text()
                || vpnInfo.m_vpnKeyDir != m_keyDirectionCombox->currentData().toString()
                || vpnInfo.m_localAddress != m_localIpEdit->text()
                || vpnInfo.m_remoteAddress != m_localIpEdit->text()) {

            isChanged = true;
        }
    }

    return isChanged;
}

bool VpnConfigPage::checkStrongSwanIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;
    int currentAuthMethod = m_authModeComboBox->currentData().toInt();

    if (vpnInfo.m_authMethod != currentAuthMethod) {
        isChanged = true;
    }

    if (currentAuthMethod == KYAUTH_KEY) {
        if (vpnInfo.m_caCertificate != m_caCertPathEdit->text()
                || vpnInfo.m_userCertificate != m_userCertPathEdit->text()
                || vpnInfo.m_userKey != m_privateKeyEdit->text()
                || vpnInfo.m_privatePasswd != m_privateKeyPwdEdit->text()
                || vpnInfo.m_privatePasswdPolicy != m_pwdOptionCombox->currentData().toInt()) {

            isChanged = true;

        }
    }else if (currentAuthMethod == KYAUTH_AGENT) {
        if (vpnInfo.m_userCertificate != m_userCertPathEdit->text()) {
            isChanged = true;
        }
    } else if (currentAuthMethod == KYAUTH_SMARTCARD) {
        if (vpnInfo.m_pinId != m_pinEdit->text()) {
            isChanged = true;
        }
    } else if (currentAuthMethod == KYAUTH_EAP) {
        if (vpnInfo.m_userName != m_usernameEdit->text() || vpnInfo.m_userPasswd != m_userPwdEdit->text()) {
            isChanged = true;
        }
    }
    return isChanged;
}
