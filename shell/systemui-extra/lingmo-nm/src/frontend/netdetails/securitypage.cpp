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
#include "securitypage.h"
#include "netdetail.h"

#include <QFileDialog>

#define  DETAIL_MIN_LABEL_WIDTH  80
#define  DETAIL_MIN_EDIT_WIDTH  390
#define  MIN_LABEL_WIDTH  146
#define  MIN_EDIT_WIDTH  286

SecurityPage::SecurityPage(bool isNetDetailPage, QWidget *parent) : isDetailPage(isNetDetailPage), QFrame(parent)
{
    initUI();
    initConnect();
}

void SecurityPage::initUI()
{
    mainLayout = new QVBoxLayout(this);
    secuTypeLabel = new QLabel(this);
    pwdLabel = new QLabel(this);
    //企业wifi共有
    eapTypeLabel = new QLabel(this);
    //TLS
    identityLable = new QLabel(this);
    domainLable = new QLabel(this);
    caCertPathLabel = new QLabel(this);
    caNeedFlagLabel = new QLabel(this);
    clientCertPathLabel = new FixLabel(this);
    clientCertPathLabel->setFixedWidth(MIN_LABEL_WIDTH);
    clientPrivateKeyLabel = new FixLabel(this);
    clientPrivateKeyLabel->setFixedWidth(MIN_LABEL_WIDTH);
    clientPrivateKeyPwdLabel = new FixLabel(this);
    clientPrivateKeyPwdLabel->setFixedWidth(MIN_LABEL_WIDTH);
    pwdOptionLabel = new FixLabel(this);
    pwdOptionLabel->setFixedWidth(MIN_LABEL_WIDTH);

    //PEAP TTLS共有
    eapMethodLabel = new FixLabel(this);
    eapMethodLabel->setFixedWidth(MIN_LABEL_WIDTH);
    userNameLabel = new QLabel(this);
    userPwdLabel = new QLabel(this);
    userPwdFlagLabel = new QLabel(this);

    secuTypeCombox = new QComboBox(this);
    pwdEdit = new KPasswordEdit(this);
    pwdEdit->setUseCustomPalette(true);
    eapTypeCombox = new QComboBox(this);
    //TLS
    identityEdit = new LineEdit(this);
    domainEdit = new LineEdit(this);
    caCertPathCombox = new QComboBox(this);
    caNeedBox = new QCheckBox(this);
    clientCertPathCombox = new QComboBox(this);
    clientPrivateKeyCombox = new QComboBox(this);
    clientPrivateKeyPwdEdit = new KPasswordEdit(this);
    clientPrivateKeyPwdEdit->setUseCustomPalette(true);
    pwdOptionCombox = new QComboBox(this);
    tlsWidget = new QWidget(this);

    //PEAP && TTLS
    eapMethodCombox = new QComboBox(this);
    userNameEdit = new LineEdit(this);
    userPwdEdit = new KPasswordEdit(this);
    userPwdEdit->setUseCustomPalette(true);
    userPwdFlagBox = new QCheckBox(this);

    //FAST
    m_pacCheckBox = new QCheckBox(this);
    m_pacProvisionComboBox = new QComboBox(this);
    m_pacFilePathComboBox = new QComboBox(this);
    m_pacProvisionLabel = new FixLabel(this);
    m_pacProvisionLabel->setFixedWidth(MIN_LABEL_WIDTH);
    m_pacFlagLabel = new FixLabel(this);
    m_pacFileLabel = new QLabel(this);

    QWidget *queryWidget = new QWidget(this);
    QHBoxLayout *queryLayout = new QHBoxLayout(queryWidget);
    queryLayout->setContentsMargins(0, 0, 0, 0);
    queryLayout->addWidget(userPwdFlagBox);
    queryLayout->addWidget(userPwdFlagLabel);
    queryLayout->addStretch();

    //记住该网络复选框
    m_emptyLabel = new QLabel(this);
    m_emptyLabel->setMinimumWidth(MIN_LABEL_WIDTH - 8);
    m_checkLabel = new QLabel(this);
    m_checkLabel->setText(tr("Remember the Network")); //记住该网络
    m_rememberCheckBox = new QCheckBox(this);
    m_rememberCheckBox->setChecked(true);
    QWidget *checkWidget = new QWidget(this);
    QHBoxLayout *rememberLayout = new QHBoxLayout(checkWidget);
    rememberLayout->setContentsMargins(0, 0, 0, 0);
    rememberLayout->addWidget(m_emptyLabel);
    rememberLayout->addWidget(m_rememberCheckBox);
    rememberLayout->addWidget(m_checkLabel);
    rememberLayout->addStretch();

    //允许自动PAC配置复选框
    m_pacCheckWidget = new QWidget(this);
    QGridLayout *pacCheckLayout = new QGridLayout(m_pacCheckWidget);
    pacCheckLayout->setContentsMargins(0, 0, 0, 0);
    pacCheckLayout->setVerticalSpacing(0);
    pacCheckLayout->setColumnMinimumWidth(0, 16);
    pacCheckLayout->addWidget(m_pacProvisionComboBox, 0, 0, 1, 2);
    pacCheckLayout->addWidget(m_pacCheckBox, 1, 0);
    pacCheckLayout->addWidget(m_pacFlagLabel, 1, 1);

    topLayout = new QGridLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setVerticalSpacing(16);
    // 安全 Label和选项框 第0行，第0列，第1列
    topLayout->addWidget(secuTypeLabel, 0, 0);
    topLayout->addWidget(secuTypeCombox, 0, 1);
    //密码 Label和密码框 第1行，第0列，第1列
    topLayout->addWidget(pwdLabel, 1, 0);
    topLayout->addWidget(pwdEdit, 1, 1);
    // EAP认证 Label和选项框 第2行，第0列，第1列
    topLayout->addWidget(eapTypeLabel, 2, 0);
    topLayout->addWidget(eapTypeCombox, 2, 1);
    // 匿名身份 Label和输入框 第3行，第0列，第1列
    topLayout->addWidget(identityLable, 3, 0);
    topLayout->addWidget(identityEdit, 3, 1);
    //PAC配置 允许自动PAC配置 第4行，第0列；第4行,第1列，占2行1列
    topLayout->addWidget(m_pacProvisionLabel, 4, 0, Qt::AlignTop);
    topLayout->addWidget(m_pacCheckWidget, 4, 1);
    // PAC文件
    topLayout->addWidget(m_pacFileLabel, 6, 0);
    topLayout->addWidget(m_pacFilePathComboBox, 6, 1);
    //内部认证 Label和选项框
    topLayout->addWidget(eapMethodLabel, 7, 0);
    topLayout->addWidget(eapMethodCombox, 7, 1);
    //用户名 Label和输入框
    topLayout->addWidget(userNameLabel, 8, 0);
    topLayout->addWidget(userNameEdit, 8, 1);
    //密码 Label和密码框
    topLayout->addWidget(userPwdLabel, 9, 0);
    topLayout->addWidget(userPwdEdit, 9, 1);

    // CA证书选项框及CheckBox布局
    QWidget *caWidget = new QWidget(this);
    QGridLayout *checkLayout = new QGridLayout(caWidget);
    checkLayout->setContentsMargins(0, 0, 0, 0);
    checkLayout->setVerticalSpacing(0);
    checkLayout->setColumnMinimumWidth(0, 16);
    checkLayout->addWidget(caCertPathCombox, 0, 0, 1, 2);
    checkLayout->addWidget(caNeedBox, 1, 0);
    checkLayout->addWidget(caNeedFlagLabel, 1, 1);

    bottomLayout = new QGridLayout(tlsWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setVerticalSpacing(8);
    bottomLayout->setHorizontalSpacing(0);
    // 域 Label和输入框 第0行，第0列，第1列
    bottomLayout->addWidget(domainLable, 0, 0);
    bottomLayout->addWidget(domainEdit, 0, 1);

    // CA证书 Label第1行，第0列
    bottomLayout->addWidget(caCertPathLabel, 1, 0);
    // CA证书选项框 不需要CA证书复选框 从第1行，第1列开始，占2行1列
    bottomLayout->addWidget(caWidget, 1, 1, 2, 1);
    // 用户证书 Label和选项框 第3行，第0列，第1列
    bottomLayout->addWidget(clientCertPathLabel, 3, 0);
    bottomLayout->addWidget(clientCertPathCombox, 3, 1);
    // 用户私钥 Label和选项框 第4行，第0列，第1列
    bottomLayout->addWidget(clientPrivateKeyLabel, 4, 0);
    bottomLayout->addWidget(clientPrivateKeyCombox, 4, 1);
    // 私钥密码 Label和密码框 第5行，第0列，第1列
    bottomLayout->addWidget(clientPrivateKeyPwdLabel, 5, 0);
    bottomLayout->addWidget(clientPrivateKeyPwdEdit, 5, 1);
    // 密码选项 Label和选项框 第6行，第0列，第1列
    bottomLayout->addWidget(pwdOptionLabel, 6, 0);
    bottomLayout->addWidget(pwdOptionCombox, 6, 1);

    if (isDetailPage) {
        checkWidget->hide();
        topLayout->addWidget(queryWidget, 10, 1);
        changeColumnWidthWithSecuType();
        m_pacFlagLabel->setMinimumWidth(MIN_EDIT_WIDTH);
    } else {
        queryWidget->hide();
        topLayout->setColumnMinimumWidth(0, MIN_LABEL_WIDTH);
        topLayout->setColumnMinimumWidth(1, MIN_EDIT_WIDTH);
        bottomLayout->setColumnMinimumWidth(0, MIN_LABEL_WIDTH - 8);
        m_pacFlagLabel->setMinimumWidth(MIN_EDIT_WIDTH - 36);
    }

    topLayout->addWidget(tlsWidget, 10, 0, 6, 2);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(checkWidget);
    mainLayout->addStretch();

    secuTypeLabel->setText(tr("Security"));
    pwdLabel->setText(tr("Password"));
    pwdEdit->setPlaceholderText(hintRequired);

    //企业wifi共有
    eapTypeLabel->setText(tr("EAP type"));
    //TLS
    identityLable->setText(tr("Identity"));
    domainLable->setText(tr("Domain"));
    caCertPathLabel->setText(tr("CA certficate"));
    caNeedFlagLabel->setText(tr("no need for CA certificate"));
    clientCertPathLabel->setLabelText(tr("User certificate"));
    clientPrivateKeyLabel->setLabelText(tr("User private key"));
    clientPrivateKeyPwdLabel->setLabelText(tr("User key password"));
    pwdOptionLabel->setLabelText(tr("Password options"));
    identityEdit->setPlaceholderText(tr("Required"));
    clientPrivateKeyPwdEdit->setPlaceholderText(hintRequired);

    //PEAP TTLS共有
    eapMethodLabel->setLabelText(tr("Ineer authentication"));
    userNameLabel->setText(tr("Username"));
    userPwdLabel->setText(tr("Password"));
    userPwdFlagLabel->setText(tr("Ask pwd each query"));
    userNameEdit->setPlaceholderText(tr("Required"));
    userPwdEdit->setPlaceholderText(hintRequired);

    secuTypeCombox->addItem(tr("None"),NONE);
    secuTypeCombox->addItem(tr("WPA&WPA2 Personal"),WPA_AND_WPA2_PERSONAL);
    secuTypeCombox->addItem(tr("WPA&WPA2 Enterprise"), WPA_AND_WPA2_ENTERPRISE);
    secuTypeCombox->addItem(tr("WPA3 Personal"), WPA3_PERSONAL);

    eapTypeCombox->addItem("TLS", TLS);
    eapTypeCombox->addItem("PEAP", PEAP);
    eapTypeCombox->addItem("TTLS", TTLS);
    eapTypeCombox->addItem("LEAP", LEAP);
    eapTypeCombox->addItem("PWD", PWD);
    eapTypeCombox->addItem("FAST", FAST);
    eapTypeCombox->setCurrentIndex(TLS);
    //TLS
    caCertPathCombox->addItem(tr("None"), QString(tr("None"))); //无
    caCertPathCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...

    clientCertPathCombox->addItem(tr("None"), QString(tr("None"))); //无
    clientCertPathCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...

    clientPrivateKeyCombox->addItem(tr("None"), QString(tr("None"))); //无
    clientPrivateKeyCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...

    //仅为该用户存储密码
    pwdOptionCombox->addItem(tr("Store passwords only for this user"), QString(tr("Store password only for this user")));
    //存储所有用户的密码
    pwdOptionCombox->addItem(tr("Store passwords for all users"), QString(tr("Store password for all users")));
    //每次询问这个密码
    pwdOptionCombox->addItem(tr("Ask this password every time"), QString(tr("Ask password every time")));
    pwdOptionCombox->setCurrentIndex(1);

    //FAST
    m_pacCheckBox->setChecked(true);
    m_pacProvisionLabel->setLabelText(tr("PAC provisioning")); //PAC配置
    m_pacFlagLabel->setLabelText(tr("Allow automatic PAC provisioning")); //允许自动PAC配置
    m_pacFileLabel->setText(tr("PAC file")); //PAC文件
    m_pacProvisionComboBox->addItem(tr("Anonymous"), ANON); //匿名
    m_pacProvisionComboBox->addItem(tr("Authenticated"), AUTHEN); //已认证
    m_pacProvisionComboBox->addItem(tr("Both"), BOTH); //两者兼用
    m_pacProvisionComboBox->setCurrentIndex(ANON);
    m_pacFilePathComboBox->addItem(tr("None"), QString(tr("None"))); //无
    m_pacFilePathComboBox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...

    //禁用ClearBtn按钮
    pwdEdit->setClearButtonEnabled(false);
    clientPrivateKeyPwdEdit->setClearButtonEnabled(false);
    userPwdEdit->setClearButtonEnabled(false);

    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    pwdEdit->setValidator(latitude);
    clientPrivateKeyPwdEdit->setValidator(latitude);
    userPwdEdit->setValidator(latitude);

    showNone();
}

void SecurityPage::initConnect()
{
    //安全类型变化
//    connect(secuTypeCombox, &QComboBox::currentTextChanged, this, &SecurityPage::onSecuTypeComboxIndexChanged);
    connect(secuTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SecurityPage::onSecuTypeComboxIndexChanged);

    connect(secuTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SecurityPage::changeColumnWidthWithSecuType);
    //EAP方式变化
//    connect(eapTypeCombox, &QComboBox::currentTextChanged, this, &SecurityPage::onEapTypeComboxIndexChanged);
    connect(eapTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SecurityPage::onEapTypeComboxIndexChanged);

    connect(caNeedBox, &QCheckBox::clicked, this, &SecurityPage::onCaNeedBoxClicked);

    connect(caCertPathCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SecurityPage::onCaCertPathComboxIndexChanged);

    connect(clientCertPathCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SecurityPage::onClientCertPathComboxIndexChanged);

    connect(clientPrivateKeyCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SecurityPage::onClientPrivateKeyComboxIndexChanged);

    connect(pwdOptionCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SecurityPage::onPwdOptionComboxIndexChanged);

    connect(secuTypeCombox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(pwdEdit, &LineEdit::textChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(eapTypeCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(identityEdit, &LineEdit::textChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(caCertPathCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(caNeedBox, &QCheckBox::stateChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(clientCertPathCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(clientPrivateKeyCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(clientPrivateKeyPwdEdit, &LineEdit::textChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(eapMethodCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(userNameEdit, &LineEdit::textChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(userPwdEdit, &LineEdit::textChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(m_pacCheckBox, &QCheckBox::stateChanged, this, &SecurityPage::setEnableOfSaveBtn);
    connect(m_pacProvisionComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_pacFilePathComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));

    connect(m_pacCheckBox, &QCheckBox::clicked, this, &SecurityPage::onPacBoxClicked);
    connect(m_pacFilePathComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SecurityPage::onPacFilePathComboxIndexChanged);
}

void SecurityPage::setSecurity(KySecuType index)
{
    secuTypeCombox->setCurrentIndex(index);
    onSecuTypeComboxIndexChanged();
}

void SecurityPage::setPsk(const QString &psk)
{
    pwdEdit->setText(psk);
}

void SecurityPage::setTlsInfo(KyEapMethodTlsInfo &info)
{
    showTls();
    identityEdit->setText(info.identity);
    domainEdit->setText(info.domain);
    if (info.caCertPath.isEmpty()) {
        caCertPathCombox->setItemText(0, QString(tr("None")));
        caNeedBox->setChecked(true);
        caCertPathCombox->setEnabled(false);
    } else {
        caCertPathCombox->setItemText(0, info.caCertPath);
        caNeedBox->setChecked(false);
        caCertPathCombox->setEnabled(true);
    }

    if (info.clientCertPath.isEmpty()) {
        clientCertPathCombox->setItemText(0, "");
    } else {
        clientCertPathCombox->setItemText(0, info.clientCertPath);
    }

    if (info.clientPrivateKey.isEmpty()) {
        clientPrivateKeyCombox->setItemText(0, "");
    } else {
        clientPrivateKeyCombox->setItemText(0, info.clientPrivateKey);
    }

    clientPrivateKeyPwdEdit->setText(info.clientPrivateKeyPWD);

    if (info.m_privateKeyPWDFlag == NetworkManager::Setting::AgentOwned) {
        pwdOptionCombox->setCurrentIndex(0);
    } else if (info.m_privateKeyPWDFlag == NetworkManager::Setting::None) {
        pwdOptionCombox->setCurrentIndex(1);
    } else {
        pwdOptionCombox->setCurrentIndex(2);
    }
}

void SecurityPage::setPeapInfo(KyEapMethodPeapInfo &info)
{
    showPeapOrTtls();
    eapTypeCombox->setCurrentIndex(PEAP);
    onEapTypeComboxIndexChanged();
    if (info.phase2AuthMethod == KyAuthMethodMschapv2) {
        eapMethodCombox->setCurrentIndex(MSCHAPV2_PEAP);
    } else if (info.phase2AuthMethod == KyAuthMethodMd5){
        eapMethodCombox->setCurrentIndex(MD5_PEAP);
    } else if (info.phase2AuthMethod == KyAuthMethodGtc) {
        eapMethodCombox->setCurrentIndex(GTC_PEAP);
    }
    userNameEdit->setText(info.userName);
    userPwdEdit->setText(info.userPWD);
    if (info.m_passwdFlag) {
        userPwdFlagBox->setChecked(true);
    } else {
        userPwdFlagBox->setChecked(false);
    }
}

void SecurityPage::setTtlsInfo(KyEapMethodTtlsInfo &info)
{
    showPeapOrTtls();
    eapTypeCombox->setCurrentIndex(TTLS);
    onEapTypeComboxIndexChanged();

    if (info.authType == AUTH_EAP) {
        if (info.authEapMethod = KyAuthEapMethodMschapv2) {
            eapMethodCombox->setCurrentIndex(MSCHAPV2_EAP);
        } else if (info.authEapMethod = KyAuthEapMethodMd5) {
            eapMethodCombox->setCurrentIndex(MD5_EAP);
        } else if (info.authEapMethod = KyAuthEapMethodMd5) {
            eapMethodCombox->setCurrentIndex(MD5_EAP);
        } else {
            qDebug() << "not support yet. AUTH_EAP method" << info.authEapMethod;
        }
    } else {
        if (info.authNoEapMethod == KyAuthMethodPap) {
            eapMethodCombox->setCurrentIndex(PAP);
        } else if (info.authNoEapMethod == KyAuthMethodMschap) {
            eapMethodCombox->setCurrentIndex(MSCHAP);
        } else if (info.authNoEapMethod == KyAuthMethodMschapv2) {
            eapMethodCombox->setCurrentIndex(MSCHAPV2);
        } else if (info.authNoEapMethod == KyAuthMethodChap) {
            eapMethodCombox->setCurrentIndex(CHAP);
        } else {
            qDebug() << "not support yet. AUTH_NO_EAP method" << info.authNoEapMethod;
        }
    }
    userNameEdit->setText(info.userName);
    userPwdEdit->setText(info.userPWD);
    if (info.m_passwdFlag) {
        userPwdFlagBox->setChecked(true);
    } else {
        userPwdFlagBox->setChecked(false);
    }
}

void SecurityPage::setLeapInfo(KyEapMethodLeapInfo &info)
{
    showLeapOrPwd();
    eapTypeCombox->setCurrentIndex(LEAP);
    userNameEdit->setText(info.m_userName);
    userPwdEdit->setText(info.m_userPwd);
    if (info.m_passwdFlag) {
        userPwdFlagBox->setChecked(true);
    } else {
        userPwdFlagBox->setChecked(false);
    }
}

void SecurityPage::setPwdInfo(KyEapMethodPwdInfo &info)
{
    showLeapOrPwd();
    eapTypeCombox->setCurrentIndex(PWD);
    userNameEdit->setText(info.m_userName);
    userPwdEdit->setText(info.m_userPwd);
    if (info.m_passwdFlag) {
        userPwdFlagBox->setChecked(true);
    } else {
        userPwdFlagBox->setChecked(false);
    }
}

void SecurityPage::setFastInfo(KyEapMethodFastInfo &info)
{
    showFast();
    eapTypeCombox->setCurrentIndex(FAST);
    onEapTypeComboxIndexChanged();

    identityEdit->setText(info.m_anonIdentity);
    m_pacCheckBox->setChecked(false);
    m_pacProvisionComboBox->setCurrentIndex(ANON);
    if (info.m_pacProvisioning == KyFastProvisioningDisabled) {
        m_pacCheckBox->setChecked(false);
        m_pacProvisionComboBox->setCurrentIndex(ANON);
        m_pacProvisionComboBox->setEnabled(false);
    } else if (info.m_pacProvisioning == KyFastProvisioningAllowUnauthenticated) {
        m_pacProvisionComboBox->setCurrentIndex(ANON);
    } else if (info.m_pacProvisioning == KyFastProvisioningAllowAuthenticated) {
        m_pacProvisionComboBox->setCurrentIndex(AUTHEN);
    } else if (info.m_pacProvisioning == KyFastProvisioningAllowBoth) {
        m_pacProvisionComboBox->setCurrentIndex(BOTH);
    } else {
        qWarning() << "Unknown fastProvisioning type" << Q_FUNC_INFO << __LINE__;
        m_pacCheckBox->setChecked(false);
        m_pacProvisionComboBox->setEnabled(false);
    }

    if (info.m_pacFilePath.isEmpty()) {
        m_pacFilePathComboBox->setItemText(0, QString(tr("None")));
    } else {
        m_pacFilePathComboBox->setItemText(0, info.m_pacFilePath);
    }

    if (info.m_authMethod == KyAuthMethodGtc) {
        eapMethodCombox->setCurrentIndex(GTC_FAST);
    } else if (info.m_authMethod == KyAuthMethodMschapv2) {
        eapMethodCombox->setCurrentIndex(MSCHAPV2_FAST);
    }
    userNameEdit->setText(info.m_userName);
    userPwdEdit->setText(info.m_userPwd);
    if (info.m_passwdFlag) {
        userPwdFlagBox->setChecked(true);
    } else {
        userPwdFlagBox->setChecked(false);
    }
}

void SecurityPage::setSecurityVisible(const bool &visible)
{
    if (secuTypeLabel) {
        secuTypeLabel->setVisible(visible);
    } else {
        qWarning() << "Set visible of secuTypeLabel failed because of null pointer" << Q_FUNC_INFO << __LINE__;
    }
    if (secuTypeCombox) {
        secuTypeCombox->setVisible(visible);
    } else {
        qWarning() << "Set visible of secuTypeCombox failed because of null pointer" << Q_FUNC_INFO << __LINE__;
    }
}

void SecurityPage::updateTlsChange(KyEapMethodTlsInfo &info)
{
    KyEapMethodTlsInfo tlsInfo = assembleTlsInfo();
    if (tlsInfo.clientPrivateKeyPWD != info.clientPrivateKeyPWD) {
        tlsInfo.bChanged = true;
    }
    tlsInfo.devIfaceName = info.devIfaceName;
    info = tlsInfo;
}

void SecurityPage::updatePeapChange(KyEapMethodPeapInfo &info)
{
    KyEapMethodPeapInfo peapInfo = assemblePeapInfo();
    if (peapInfo.userPWD != info.userPWD) {
        peapInfo.bChanged = true;
    }
    info = peapInfo;
}

void SecurityPage::updateTtlsChange(KyEapMethodTtlsInfo &info)
{
    KyEapMethodTtlsInfo ttlsInfo = assembleTtlsInfo();
    if (ttlsInfo.userPWD != info.userPWD) {
        ttlsInfo.bChanged = true;
    }
    info = ttlsInfo;
}

void SecurityPage::updateLeapChange(KyEapMethodLeapInfo &info)
{
    KyEapMethodLeapInfo leapInfo = assembleLeapInfo();
    if (leapInfo.m_userPwd != info.m_userPwd) {
        leapInfo.bChanged = true;
    }
    info = leapInfo;
}

void SecurityPage::updatePwdChange(KyEapMethodPwdInfo &info)
{
    KyEapMethodPwdInfo pwdInfo = assemblePwdInfo();
    if (pwdInfo.m_userPwd != info.m_userPwd) {
        pwdInfo.bChanged = true;
    }
    info = pwdInfo;
}

void SecurityPage::updateFastChange(KyEapMethodFastInfo &info)
{
    KyEapMethodFastInfo pwdInfo = assembleFastInfo();
    if (pwdInfo.m_userPwd != info.m_userPwd) {
        pwdInfo.bChanged = true;
    }
    info = pwdInfo;
}

void SecurityPage::getSecuType(KySecuType &secuType, KyEapMethodType &enterpriseType)
{
    secuType = (KySecuType)secuTypeCombox->currentData().toInt();
    enterpriseType = (KyEapMethodType)eapTypeCombox->currentData().toInt();
}

bool SecurityPage::getAutoConnectState()
{
    bool state = m_rememberCheckBox->isChecked();
    return state;
}

bool SecurityPage::checkIsChanged(const ConInfo info)
{
    if (info.secType != secuTypeCombox->currentData().toInt()) {
        return true;
    } else {
        if (info.secType == NONE) {
            return false;
        } else if (info.secType == WPA_AND_WPA2_PERSONAL || info.secType == WPA3_PERSONAL) {
                return !(info.strPassword == pwdEdit->text());
        } else {
            if (info.enterpriseType != eapTypeCombox->currentData().toInt()) {
                return true;
            } else {
                if (info.enterpriseType == TLS) {
                    return !(info.tlsInfo == assembleTlsInfo());
                } else if (info.enterpriseType == PEAP) {
                    return !(info.peapInfo == assemblePeapInfo());
                } else if (info.enterpriseType == TTLS) {
                    return !(info.ttlsInfo == assembleTtlsInfo());
                } else if (info.enterpriseType == LEAP) {
                    return !(info.leapInfo == assembleLeapInfo());
                } else if (info.enterpriseType == PWD) {
                    return !(info.pwdInfo == assemblePwdInfo());
                } else if (info.enterpriseType == FAST) {
                    return !(info.fastInfo == assembleFastInfo());
                }
            }
        }
    }
}

void SecurityPage::showNone()
{
    pwdLabel->hide();
    pwdEdit->hide();
    //企业wifi共有
    eapTypeLabel->hide();
    eapTypeCombox->hide();

    //TLS
    identityLable->hide();
    identityEdit->hide();
    tlsWidget->hide();

    //PEAP TTLS共有
    eapMethodLabel->hide();
    userNameLabel->hide();
    userPwdLabel->hide();
    userPwdFlagBox->hide();

    eapMethodCombox->hide();
    userNameEdit->hide();
    userPwdEdit->hide();
    userPwdFlagLabel->hide();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void SecurityPage::showPsk()
{
    pwdLabel->show();
    pwdEdit->show();
    //企业wifi共有
    eapTypeLabel->hide();
    eapTypeCombox->hide();

    //TLS
    identityLable->hide();
    identityEdit->hide();
    tlsWidget->hide();

    //PEAP TTLS共有
    eapMethodLabel->hide();
    userNameLabel->hide();
    userPwdLabel->hide();
    userPwdFlagBox->hide();

    eapMethodCombox->hide();
    userNameEdit->hide();
    userPwdEdit->hide();
    userPwdFlagLabel->hide();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void SecurityPage::showTls()
{
    pwdLabel->hide();
    pwdEdit->hide();
    eapTypeCombox->show();
    eapTypeLabel->show();

    //TLS
    identityLable->show();
    identityEdit->show();
    tlsWidget->show();

    //PEAP TTLS共有
    eapMethodLabel->hide();
    userNameLabel->hide();
    userPwdLabel->hide();
    userPwdFlagBox->hide();

    eapMethodCombox->hide();
    userNameEdit->hide();
    userPwdEdit->hide();
    userPwdFlagLabel->hide();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void SecurityPage::showPeapOrTtls()
{
    pwdLabel->hide();
    pwdEdit->hide();

    //企业wifi共有
    eapTypeLabel->show();
    eapTypeCombox->show();

    //TLS
    identityLable->hide();
    identityEdit->hide();
    tlsWidget->hide();

    //PEAP TTLS共有
    eapMethodLabel->show();
    userNameLabel->show();
    userPwdLabel->show();
    userPwdFlagBox->show();

    eapMethodCombox->show();
    userNameEdit->show();
    userPwdEdit->show();
    userPwdFlagLabel->show();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void SecurityPage::showLeapOrPwd()
{
    pwdLabel->hide();
    pwdEdit->hide();

    //企业wifi共有
    eapTypeLabel->show();
    eapTypeCombox->show();

    //TLS
    identityLable->hide();
    identityEdit->hide();
    tlsWidget->hide();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();

    eapMethodLabel->hide();
    eapMethodCombox->hide();
    userPwdFlagBox->show();
    userPwdFlagLabel->show();

    userNameLabel->show();
    userNameEdit->show();
    userPwdLabel->show();
    userPwdEdit->show();
}

void SecurityPage::showFast()
{
    pwdLabel->hide();
    pwdEdit->hide();

    //企业wifi共有
    eapTypeLabel->show();
    eapTypeCombox->show();

    //TLS
    identityLable->show();
    identityEdit->show();
    tlsWidget->hide();

    //FAST
    m_pacProvisionLabel->show();
    m_pacCheckWidget->show();
    m_pacFileLabel->show();
    m_pacFilePathComboBox->show();

    eapMethodLabel->show();
    eapMethodCombox->show();
    userPwdFlagBox->show();
    userPwdFlagLabel->show();

    userNameLabel->show();
    userNameEdit->show();
    userPwdLabel->show();
    userPwdEdit->show();
}

KyEapMethodTlsInfo SecurityPage::assembleTlsInfo()
{
    KyEapMethodTlsInfo info;
    info.identity = identityEdit->text();
    info.domain = domainEdit->text();
    info.caCertPath = caCertPathCombox->currentText();
    info.bNeedCa = !caNeedBox->isChecked();
    info.clientCertPath = clientCertPathCombox->currentText();
    info.clientPrivateKey = clientPrivateKeyCombox->currentText();
    info.clientPrivateKeyPWD = clientPrivateKeyPwdEdit->text();
    switch (pwdOptionCombox->currentIndex()) {
    case 0:
        info.m_privateKeyPWDFlag = NetworkManager::Setting::AgentOwned;
        break;
    case 1:
        info.m_privateKeyPWDFlag = NetworkManager::Setting::None;
        break;
    case 2:
        info.m_privateKeyPWDFlag = NetworkManager::Setting::NotSaved;
        break;
    default:
        break;
    }
    return info;
}

KyEapMethodPeapInfo SecurityPage::assemblePeapInfo()
{
    KyEapMethodPeapInfo info;
//    info.phase2AuthMethod = (KyNoEapMethodAuth)eapMethodCombox->currentData().toInt();
    switch (eapMethodCombox->currentIndex()) {
    case 0:
        info.phase2AuthMethod = KyAuthMethodMschapv2;
        break;
    case 1:
        info.phase2AuthMethod = KyAuthMethodMd5;
        break;
    case 2:
        info.phase2AuthMethod = KyAuthMethodGtc;
        break;
    default:
        break;
    }
    info.userName = userNameEdit->text();
    info.userPWD = userPwdEdit->text();
    info.m_passwdFlag = (userPwdFlagBox->isChecked() ? NetworkManager::Setting::NotSaved : NetworkManager::Setting::None);

    return info;
}
KyEapMethodTtlsInfo SecurityPage::assembleTtlsInfo()
{
    KyEapMethodTtlsInfo info;
    switch (eapMethodCombox->currentIndex()) {
    case PAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodPap;
        break;
    case MSCHAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodChap;
        break;
    case MSCHAPV2_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodMschapv2;
        break;
    case MSCHAPV2:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodMschapv2;
        break;
    case CHAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodChap;
        break;
    case MD5_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodMd5;
        break;
    case GTC_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodGtc;
        break;
    default:
        break;
    }
    info.userName = userNameEdit->text();
    info.userPWD = userPwdEdit->text();
    info.m_passwdFlag = (userPwdFlagBox->isChecked() ? NetworkManager::Setting::NotSaved : NetworkManager::Setting::None);
    return info;
}

KyEapMethodLeapInfo SecurityPage::assembleLeapInfo()
{
    KyEapMethodLeapInfo info;
    info.m_userName = userNameEdit->text();
    info.m_userPwd = userPwdEdit->text();
    info.m_passwdFlag = (userPwdFlagBox->isChecked() ? NetworkManager::Setting::NotSaved : NetworkManager::Setting::None);
    return info;
}

KyEapMethodPwdInfo SecurityPage::assemblePwdInfo()
{
    KyEapMethodPwdInfo info;
    info.m_userName = userNameEdit->text();
    info.m_userPwd = userPwdEdit->text();
    info.m_passwdFlag = (userPwdFlagBox->isChecked() ? NetworkManager::Setting::NotSaved : NetworkManager::Setting::None);
    return info;
}

KyEapMethodFastInfo SecurityPage::assembleFastInfo()
{
    KyEapMethodFastInfo info;
       info.m_anonIdentity = identityEdit->text();
       switch (m_pacProvisionComboBox->currentIndex()) {
       case ANON:
           info.m_pacProvisioning = KyFastProvisioningAllowUnauthenticated;
           break;
       case AUTHEN:
           info.m_pacProvisioning = KyFastProvisioningAllowAuthenticated;
           break;
       case BOTH:
           info.m_pacProvisioning = KyFastProvisioningAllowBoth;
           break;
       default:
           break;
       }
       info.m_allowAutoPacFlag = m_pacCheckBox->isChecked();
       info.m_pacFilePath = m_pacFilePathComboBox->currentText();

       switch (eapMethodCombox->currentIndex()) {
       case GTC_FAST:
           info.m_authMethod = KyAuthMethodGtc;
           break;
       case MSCHAPV2_FAST:
           info.m_authMethod = KyAuthMethodMschapv2;
           break;
       default:
           break;
       }
       info.m_userName = userNameEdit->text();
       info.m_userPwd = userPwdEdit->text();
       info.m_passwdFlag = (userPwdFlagBox->isChecked() ? NetworkManager::Setting::NotSaved : NetworkManager::Setting::None);
       return info;
}

void SecurityPage::updateSecurityChange(KyWirelessConnectSetting &setting)
{
    qDebug() << "secuTypeCombox->currentData()" << secuTypeCombox->currentData().toInt() << pwdEdit->text();
    if (secuTypeCombox->currentData().toInt() == NONE) {
        setting.m_psk = "";
    } else {
        setting.m_psk = pwdEdit->text();
    }

    if (secuTypeCombox->currentData().toInt() == NONE) {
        setting.m_type = WpaNone;
    } else if (secuTypeCombox->currentData().toInt() == WPA_AND_WPA2_PERSONAL) {
        setting.m_type = WpaPsk;
    } else if (secuTypeCombox->currentData().toInt() == WPA3_PERSONAL) {
        setting.m_type = SAE;
    }
    setting.isAutoConnect = m_rememberCheckBox->isChecked();
}

bool SecurityPage::checkConnectBtnIsEnabled()
{
    int index = secuTypeCombox->currentData().toInt();
    if (index == NONE) {

    } else if (index == WPA_AND_WPA2_PERSONAL || index == WPA3_PERSONAL) {
        if (pwdEdit->text().isEmpty() || pwdEdit->text().length() < 8 ) {
            qDebug() << "password is empty or length < 8";
            return false;
        }
    }  else if (index == WPA_AND_WPA2_ENTERPRISE) {
        int type = eapTypeCombox->currentData().toInt();
        if (type == TLS) {
            if (identityEdit->text().isEmpty()) {
                qDebug() << "tls identity is empty";
                return false;
            }
            QFile cafile(caCertPathCombox->currentText());
            if(!caNeedBox->isChecked() && !cafile.exists()) {
                qDebug() << "ca cert filepath " << caCertPathCombox->currentText() << " is invalid";
                return false;
            }

            QFile cliCafile(clientCertPathCombox->currentText());
            if(!cliCafile.exists()) {
                qDebug() << "client cert filepath " << clientCertPathCombox->currentText() << " is invalid";
                return false;
            }

            QFile cliKeyfile(clientPrivateKeyCombox->currentText());
            if(!cliKeyfile.exists()) {
                qDebug() << "client private key filepath " << clientPrivateKeyCombox->currentText() << " is invalid";
                return false;
            }

            if(clientPrivateKeyPwdEdit->text().isEmpty()) {
                qDebug() << "client Private Key password is empty";
                return false;
            }
        } else if (type == PEAP || type == TTLS || type == LEAP || type == PWD) {
            if(userNameEdit->text().isEmpty() || userPwdEdit->text().isEmpty()) {
                qDebug() << "user name or user password is empty";
                return false;
            }
        } else if (type == FAST) {
            if(!m_pacCheckBox->isChecked()) {
                if (m_pacFilePathComboBox->currentText() == QString(tr("None"))) {
                    qDebug() << "Not allow automatic PAC provisioning && pac file is empty";
                    return false;
                }
            }
            if(userNameEdit->text().isEmpty() || userPwdEdit->text().isEmpty()) {
                qDebug() << "user name or user password is empty";
                return false;
            }
        }
    }
    return true;
}

void SecurityPage::setEnableOfSaveBtn()
{
    Q_EMIT setSecuPageState(checkConnectBtnIsEnabled());
}

void SecurityPage::onSecuTypeComboxIndexChanged()
{
    int index = secuTypeCombox->currentData().toInt();
    if (index == WPA_AND_WPA2_PERSONAL) {
        showPsk();
        Q_EMIT this->secuTypeChanged(WPA_AND_WPA2_PERSONAL);
    }
    else if (index == WPA3_PERSONAL) {
        showPsk();
        Q_EMIT this->secuTypeChanged(WPA3_PERSONAL);
    } else if (index == WPA_AND_WPA2_ENTERPRISE) {
        onEapTypeComboxIndexChanged();
        Q_EMIT this->secuTypeChanged(WPA_AND_WPA2_ENTERPRISE);
    } else if (index == NONE) {
        showNone();
        Q_EMIT this->secuTypeChanged(NONE);
    }
}

void SecurityPage::onEapTypeComboxIndexChanged()
{
    qDebug() << "onEapTypeComboxIndexChanged";
    int index = eapTypeCombox->currentData().toInt();
    if (index == TLS) {
        showTls();
        Q_EMIT this->eapTypeChanged(TLS);
    } else if (index == PEAP) {
        showPeapOrTtls();
        eapMethodCombox->clear();
        eapMethodCombox->addItem("MSCHAPv2", MSCHAPV2_PEAP);
        eapMethodCombox->addItem("MD5", MD5_PEAP);
        eapMethodCombox->addItem("GTC", GTC_PEAP);
        Q_EMIT this->eapTypeChanged(PEAP);
    } else if (index == TTLS) {
        showPeapOrTtls();
        eapMethodCombox->clear();
        eapMethodCombox->addItem("pap", PAP);
        eapMethodCombox->addItem("mschap", MSCHAP);
        eapMethodCombox->addItem("mschapv2(eap)", MSCHAPV2_EAP);
        eapMethodCombox->addItem("mschapv2", MSCHAPV2);
        eapMethodCombox->addItem("chap", CHAP);
        eapMethodCombox->addItem("md5(eap)", MD5_EAP);
        eapMethodCombox->addItem("gtc(eap)", GTC_EAP);
        Q_EMIT this->eapTypeChanged(TTLS);
    } else if (index == LEAP) {
        showLeapOrPwd();
        Q_EMIT this->eapTypeChanged(LEAP);
    }  else if (index == PWD) {
        showLeapOrPwd();
        Q_EMIT this->eapTypeChanged(PWD);
    } else if (index == FAST) {
        showFast();
        eapMethodCombox->clear();
        eapMethodCombox->addItem("GTC", GTC_FAST);
        eapMethodCombox->addItem("MSCHAPv2", MSCHAPV2_FAST);
        Q_EMIT this->eapTypeChanged(FAST);
    }
}

void SecurityPage::onCaNeedBoxClicked()
{
    if (caNeedBox->isChecked()) {
        caCertPathCombox->setItemText(0, QString(tr("None")));
        caCertPathCombox->setEnabled(false);
    }  else {
        caCertPathCombox->setEnabled(true);
    }
}

void SecurityPage::onPacBoxClicked()
{
    if (!m_pacCheckBox->isChecked()) {
        m_pacProvisionComboBox->setCurrentIndex(ANON);
        m_pacProvisionComboBox->setEnabled(false);
    }  else {
        m_pacProvisionComboBox->setEnabled(true);
    }
}

void SecurityPage::onCaCertPathComboxIndexChanged(QString str)
{
    if (str.contains("Choose from file...") || str.contains("从文件选择..."))
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                        tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
        if (!fileName.isNull()) {
            QStringList nameList = fileName.split("/");
            caCertPathCombox->blockSignals(true);
            caCertPathCombox->setItemText(0, fileName);
            caCertPathCombox->setCurrentIndex(0);
            caCertPathCombox->blockSignals(false);
        } else {
            caCertPathCombox->blockSignals(true);
            caCertPathCombox->setItemText(0, tr("None"));
            caCertPathCombox->setCurrentIndex(0);
            caCertPathCombox->blockSignals(false);
        }
    } else {
        qWarning() << "Choose file is null or unvalible";
    }
}

void SecurityPage::onClientCertPathComboxIndexChanged(QString str)
{
    if (str.contains("Choose from file...") || str.contains("从文件选择..."))
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                        tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
        if (!fileName.isNull()) {
            clientCertPathCombox->blockSignals(true);
            clientCertPathCombox->setItemText(0, fileName);
            clientCertPathCombox->setCurrentIndex(0);
            clientCertPathCombox->blockSignals(false);
        } else {
            clientCertPathCombox->blockSignals(true);
            clientCertPathCombox->setItemText(0, tr("None"));
            clientCertPathCombox->setCurrentIndex(0);
            clientCertPathCombox->blockSignals(false);
        }
    } else {
        qWarning() << "Choose file is null or unvalible";
    }
}

void SecurityPage::onClientPrivateKeyComboxIndexChanged(QString str)
{
    if (str.contains("Choose from file...") || str.contains("从文件选择..."))
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                        tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
        if (!fileName.isNull()) {
            QStringList nameList = fileName.split("/");
            clientPrivateKeyCombox->blockSignals(true);
            clientPrivateKeyCombox->setItemText(0, fileName);
            clientPrivateKeyCombox->setCurrentIndex(0);
            clientPrivateKeyCombox->blockSignals(false);
        } else {
            clientPrivateKeyCombox->blockSignals(true);
            clientPrivateKeyCombox->setItemText(0, tr("None"));
            clientPrivateKeyCombox->setCurrentIndex(0);
            clientPrivateKeyCombox->blockSignals(false);
        }
    } else {
        qWarning() << "Choose file is null or unvalible";
    }
}

void SecurityPage::onPwdOptionComboxIndexChanged(QString str)
{
    KyEapMethodTlsInfo info;
    if (str.contains("Store passwords only for this user") || str.contains("仅为该用户存储密码")) {
        info.m_privateKeyPWDFlag = NetworkManager::Setting::AgentOwned;
        clientPrivateKeyPwdEdit->setPlaceholderText(emptyhint);
    } else if (str.contains("Store passwords for all users") || str.contains("存储所有用户的密码")) {
        info.m_privateKeyPWDFlag = NetworkManager::Setting::None;
        clientPrivateKeyPwdEdit->setPlaceholderText(hintRequired);
    } else {
        info.m_privateKeyPWDFlag = NetworkManager::Setting::NotSaved;
        clientPrivateKeyPwdEdit->setPlaceholderText(emptyhint);
    }
}

void SecurityPage::changeColumnWidthWithSecuType()
{
    if (!isDetailPage) {
        return;
    }
    if (secuTypeCombox->currentData().toInt() == WPA_AND_WPA2_ENTERPRISE &&
            eapMethodCombox->currentData().toInt() == TLS) {
        topLayout->setColumnMinimumWidth(0, MIN_LABEL_WIDTH);
        topLayout->setColumnMinimumWidth(1, MIN_EDIT_WIDTH);
        bottomLayout->setColumnMinimumWidth(0, MIN_LABEL_WIDTH - 8);

    } else {
        topLayout->setColumnMinimumWidth(0, DETAIL_MIN_LABEL_WIDTH);
        topLayout->setColumnMinimumWidth(1, DETAIL_MIN_EDIT_WIDTH);
    }
}

void SecurityPage::onPacFilePathComboxIndexChanged(QString str)
{
    if (str.contains("Choose from file...") || str.contains("从文件选择..."))
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a PAC file"), "recent:///",
                                                        tr("PAC Files (*.pac)"));
        if (!fileName.isNull()) {
            QStringList nameList = fileName.split("/");
            m_pacFilePathComboBox->blockSignals(true);
            m_pacFilePathComboBox->setItemText(0, fileName);
            m_pacFilePathComboBox->setCurrentIndex(0);
            m_pacFilePathComboBox->blockSignals(false);
        } else {
            m_pacFilePathComboBox->blockSignals(true);
            m_pacFilePathComboBox->setItemText(0, tr("None"));
            m_pacFilePathComboBox->setCurrentIndex(0);
            m_pacFilePathComboBox->blockSignals(false);
        }
    } else {
        qWarning() << "Choose file is null or unvalible";
    }
}
