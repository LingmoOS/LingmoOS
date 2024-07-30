/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "security802-1x.h"
#include "editlistdialog.h"
#include "listvalidator.h"
#include "ui_802-1x.h"

#include <KAcceleratorManager>
#include <KLocalizedString>

#include <QtCrypto>

Security8021x::Security8021x(const NetworkManager::Setting::Ptr &setting, Type type, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_setting(setting.staticCast<NetworkManager::Security8021xSetting>())
    , m_ui(new Ui::Security8021x)
{
    m_ui->setupUi(this);

    m_ui->fastPassword->setPasswordOptionsEnabled(true);
    m_ui->leapPassword->setPasswordOptionsEnabled(true);
    m_ui->md5Password->setPasswordOptionsEnabled(true);
    m_ui->peapPassword->setPasswordOptionsEnabled(true);
    m_ui->pwdPassword->setPasswordOptionsEnabled(true);
    m_ui->tlsPrivateKeyPassword->setPasswordOptionsEnabled(true);
    m_ui->ttlsPassword->setPasswordOptionsEnabled(true);

    if (type == WirelessWpaEap) {
        m_ui->auth->removeItem(0); // MD 5
        m_ui->stackedWidget->removeWidget(m_ui->md5Page);

        m_ui->auth->setItemData(0, NetworkManager::Security8021xSetting::EapMethodTls);
        m_ui->auth->setItemData(1, NetworkManager::Security8021xSetting::EapMethodLeap);
        m_ui->auth->setItemData(2, NetworkManager::Security8021xSetting::EapMethodPwd);
        m_ui->auth->setItemData(3, NetworkManager::Security8021xSetting::EapMethodFast);
        m_ui->auth->setItemData(4, NetworkManager::Security8021xSetting::EapMethodTtls);
        m_ui->auth->setItemData(5, NetworkManager::Security8021xSetting::EapMethodPeap);
    } else if (type == WirelessWpaEapSuiteB192) {
        // Remove all methods except TLS
        m_ui->auth->removeItem(6); // Protected EAP (PEAP)
        m_ui->auth->removeItem(5); // Tunneled TLS (TTLS)
        m_ui->auth->removeItem(4); // FAST
        m_ui->auth->removeItem(3); // PWD
        m_ui->auth->removeItem(2); // LEAP
        m_ui->auth->removeItem(0); // MD5
        m_ui->stackedWidget->removeWidget(m_ui->peapPage);
        m_ui->stackedWidget->removeWidget(m_ui->ttlsPage);
        m_ui->stackedWidget->removeWidget(m_ui->fastPage);
        m_ui->stackedWidget->removeWidget(m_ui->pwdPage);
        m_ui->stackedWidget->removeWidget(m_ui->leapPage);
        m_ui->stackedWidget->removeWidget(m_ui->md5Page);

        m_ui->auth->setItemData(0, NetworkManager::Security8021xSetting::EapMethodTls);
    } else {
        m_ui->auth->removeItem(2); // LEAP
        m_ui->stackedWidget->removeWidget(m_ui->leapPage);

        m_ui->auth->setItemData(0, NetworkManager::Security8021xSetting::EapMethodMd5);
        m_ui->auth->setItemData(1, NetworkManager::Security8021xSetting::EapMethodTls);
        m_ui->auth->setItemData(2, NetworkManager::Security8021xSetting::EapMethodPwd);
        m_ui->auth->setItemData(3, NetworkManager::Security8021xSetting::EapMethodFast);
        m_ui->auth->setItemData(4, NetworkManager::Security8021xSetting::EapMethodTtls);
        m_ui->auth->setItemData(5, NetworkManager::Security8021xSetting::EapMethodPeap);
    }

    if (type == WirelessWpaEapSuiteB192) {
        // Set TLS authentication as default
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodTls));
    } else {
        // Set PEAP authentication as default
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodPeap));
    }

    connect(m_ui->btnTlsAltSubjectMatches, &QPushButton::clicked, this, &Security8021x::altSubjectMatchesButtonClicked);
    connect(m_ui->btnTlsConnectToServers, &QPushButton::clicked, this, &Security8021x::connectToServersButtonClicked);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->auth, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &Security8021x::slotWidgetChanged);
    connect(m_ui->md5UserName, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->md5Password, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->md5Password, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsIdentity, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsCACert, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsUserCert, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsPrivateKey, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsPrivateKeyPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->tlsPrivateKeyPassword, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->leapUsername, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->leapPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->leapPassword, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->fastAllowPacProvisioning, &QCheckBox::stateChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->pacFile, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->pwdUsername, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->pwdPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->fastUsername, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->fastPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->fastPassword, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->ttlsCACert, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->ttlsUsername, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->ttlsPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->ttlsPassword, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->peapCACert, &KUrlRequester::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->peapUsername, &KLineEdit::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->peapPassword, &PasswordField::textChanged, this, &Security8021x::slotWidgetChanged);
    connect(m_ui->peapPassword, &PasswordField::passwordOptionChanged, this, &Security8021x::slotWidgetChanged);

    KAcceleratorManager::manage(this);
    connect(m_ui->stackedWidget, &QStackedWidget::currentChanged, this, &Security8021x::currentAuthChanged);

    altSubjectValidator = new QRegularExpressionValidator(
        QRegularExpression(
            QLatin1String("^(DNS:[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|EMAIL:[a-zA-Z0-9._-]+@[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|URI:[a-zA-Z0-9.+-]+:.+|)$")),
        this);
    serversValidator = new QRegularExpressionValidator(QRegularExpression(QLatin1String("^[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+$")), this);

    auto altSubjectListValidator = new ListValidator(this);
    altSubjectListValidator->setInnerValidator(altSubjectValidator);
    m_ui->leTlsSubjectMatch->setValidator(altSubjectListValidator);

    auto serverListValidator = new ListValidator(this);
    serverListValidator->setInnerValidator(serversValidator);
    m_ui->leTlsConnectToServers->setValidator(serverListValidator);

    if (setting) {
        loadConfig(setting);
    }
}

Security8021x::~Security8021x()
{
    delete m_ui;
}

void Security8021x::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::Security8021xSetting::Ptr securitySetting = setting.staticCast<NetworkManager::Security8021xSetting>();

    const QList<NetworkManager::Security8021xSetting::EapMethod> eapMethods = securitySetting->eapMethods();
    const NetworkManager::Security8021xSetting::AuthMethod phase2AuthMethod = securitySetting->phase2AuthMethod();

    if (securitySetting->passwordFlags().testFlag(NetworkManager::Setting::None)) {
        setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (securitySetting->passwordFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
        setPasswordOption(PasswordField::StoreForUser);
    } else {
        setPasswordOption(PasswordField::AlwaysAsk);
    }

    if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodMd5)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodMd5));
        m_ui->md5UserName->setText(securitySetting->identity());
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodTls)) {
        QStringList servers;
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodTls));
        m_ui->tlsIdentity->setText(securitySetting->identity());
        m_ui->tlsDomain->setText(securitySetting->domainSuffixMatch());
        m_ui->tlsUserCert->setUrl(QUrl::fromLocalFile(securitySetting->clientCertificate()));
        m_ui->tlsCACert->setUrl(QUrl::fromLocalFile(securitySetting->caCertificate()));
        m_ui->leTlsSubjectMatch->setText(securitySetting->subjectMatch());
        m_ui->leTlsAlternativeSubjectMatches->setText(securitySetting->altSubjectMatches().join(QLatin1String(", ")));
        for (const QString &match : securitySetting->altSubjectMatches()) {
            if (match.startsWith(QLatin1String("DNS:"))) {
                servers.append(match.right(match.length() - 4));
            }
        }
        m_ui->leTlsConnectToServers->setText(servers.join(QLatin1String(", ")));
        m_ui->tlsPrivateKey->setUrl(QUrl::fromLocalFile(securitySetting->privateKey()));
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodLeap)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodLeap));
        m_ui->leapUsername->setText(securitySetting->identity());
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodPwd)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodPwd));
        m_ui->pwdUsername->setText(securitySetting->identity());
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodFast)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodFast));
        m_ui->fastAnonIdentity->setText(securitySetting->anonymousIdentity());
        m_ui->fastAllowPacProvisioning->setChecked((int)securitySetting->phase1FastProvisioning() > 0);
        m_ui->pacMethod->setCurrentIndex(securitySetting->phase1FastProvisioning() - 1);
        m_ui->pacFile->setUrl(QUrl::fromLocalFile(securitySetting->pacFile()));
        if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodGtc) {
            m_ui->fastInnerAuth->setCurrentIndex(0);
        } else {
            m_ui->fastInnerAuth->setCurrentIndex(1);
        }
        m_ui->fastUsername->setText(securitySetting->identity());
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodTtls)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodTtls));
        m_ui->ttlsAnonIdentity->setText(securitySetting->anonymousIdentity());
        m_ui->ttlsDomain->setText(securitySetting->domainSuffixMatch());
        m_ui->ttlsCACert->setUrl(QUrl::fromLocalFile(securitySetting->caCertificate()));
        if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodPap) {
            m_ui->ttlsInnerAuth->setCurrentIndex(0);
        } else if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodMschap) {
            m_ui->ttlsInnerAuth->setCurrentIndex(1);
        } else if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodMschapv2) {
            m_ui->ttlsInnerAuth->setCurrentIndex(2);
        } else if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodChap) {
            m_ui->ttlsInnerAuth->setCurrentIndex(3);
        }
        m_ui->ttlsUsername->setText(securitySetting->identity());
    } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodPeap)) {
        m_ui->auth->setCurrentIndex(m_ui->auth->findData(NetworkManager::Security8021xSetting::EapMethodPeap));
        m_ui->peapAnonIdentity->setText(securitySetting->anonymousIdentity());
        m_ui->peapDomain->setText(securitySetting->domainSuffixMatch());
        m_ui->peapCACert->setUrl(QUrl::fromLocalFile(securitySetting->caCertificate()));
        m_ui->peapVersion->setCurrentIndex(securitySetting->phase1PeapVersion() + 1);
        if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodMschapv2) {
            m_ui->peapInnerAuth->setCurrentIndex(0);
        } else if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodMd5) {
            m_ui->peapInnerAuth->setCurrentIndex(1);
        } else if (phase2AuthMethod == NetworkManager::Security8021xSetting::AuthMethodGtc) {
            m_ui->peapInnerAuth->setCurrentIndex(2);
        }
        m_ui->peapUsername->setText(securitySetting->identity());
    }

    loadSecrets(setting);
}

void Security8021x::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::Security8021xSetting::Ptr securitySetting = setting.staticCast<NetworkManager::Security8021xSetting>();

    const QString password = securitySetting->password();
    const QList<NetworkManager::Security8021xSetting::EapMethod> eapMethods = securitySetting->eapMethods();

    if (!password.isEmpty()) {
        if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodMd5)) {
            m_ui->md5Password->setText(securitySetting->password());
        } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodLeap)) {
            m_ui->leapPassword->setText(securitySetting->password());
        } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodFast)) {
            m_ui->fastPassword->setText(securitySetting->password());
        } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodPwd)) {
            m_ui->pwdPassword->setText(securitySetting->password());
        } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodTtls)) {
            m_ui->ttlsPassword->setText(securitySetting->password());
        } else if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodPeap)) {
            m_ui->peapPassword->setText(securitySetting->password());
        }
    }

    if (eapMethods.contains(NetworkManager::Security8021xSetting::EapMethodTls)) {
        const QString privateKeyPassword = securitySetting->privateKeyPassword();
        if (!privateKeyPassword.isEmpty()) {
            m_ui->tlsPrivateKeyPassword->setText(securitySetting->privateKeyPassword());
        }
    }
}

QVariantMap Security8021x::setting() const
{
    NetworkManager::Security8021xSetting setting;

    NetworkManager::Security8021xSetting::EapMethod method =
        static_cast<NetworkManager::Security8021xSetting::EapMethod>(m_ui->auth->itemData(m_ui->auth->currentIndex()).toInt());

    setting.setEapMethods(QList<NetworkManager::Security8021xSetting::EapMethod>() << method);

    if (method == NetworkManager::Security8021xSetting::EapMethodMd5) {
        if (!m_ui->md5UserName->text().isEmpty()) {
            setting.setIdentity(m_ui->md5UserName->text());
        }

        if (m_ui->md5Password->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->md5Password->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }

        if (!m_ui->md5Password->text().isEmpty()) {
            setting.setPassword(m_ui->md5Password->text());
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodTls) {
        if (!m_ui->tlsIdentity->text().isEmpty()) {
            setting.setIdentity(m_ui->tlsIdentity->text());
        }

        if (!m_ui->tlsDomain->text().isEmpty()) {
            setting.setDomainSuffixMatch(m_ui->tlsDomain->text());
        }

        if (m_ui->tlsUserCert->url().isValid()) {
            setting.setClientCertificate(m_ui->tlsUserCert->url().toString().toUtf8().append('\0'));
        }

        if (m_ui->tlsCACert->url().isValid()) {
            setting.setCaCertificate(m_ui->tlsCACert->url().toString().toUtf8().append('\0'));
        }

        QStringList altsubjectmatches = m_ui->leTlsAlternativeSubjectMatches->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QString &match : m_ui->leTlsConnectToServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), Qt::SkipEmptyParts)) {
            const QString tempstr = QLatin1String("DNS:") + match;
            if (!altsubjectmatches.contains(tempstr)) {
                altsubjectmatches.append(tempstr);
            }
        }
        setting.setSubjectMatch(m_ui->leTlsSubjectMatch->text());
        setting.setAltSubjectMatches(altsubjectmatches);

        if (m_ui->tlsPrivateKey->url().isValid()) {
            setting.setPrivateKey(m_ui->tlsPrivateKey->url().toString().toUtf8().append('\0'));
        }

        if (!m_ui->tlsPrivateKeyPassword->text().isEmpty()) {
            setting.setPrivateKeyPassword(m_ui->tlsPrivateKeyPassword->text());
        }

        QCA::Initializer init;
        QCA::ConvertResult convRes;

        // Try if the private key is in pkcs12 format bundled with client certificate
        if (QCA::isSupported("pkcs12")) {
            QCA::KeyBundle keyBundle = QCA::KeyBundle::fromFile(m_ui->tlsPrivateKey->url().path(), m_ui->tlsPrivateKeyPassword->text().toUtf8(), &convRes);
            // Set client certificate to the same path as private key
            if (convRes == QCA::ConvertGood && keyBundle.privateKey().canDecrypt()) {
                setting.setClientCertificate(m_ui->tlsPrivateKey->url().toString().toUtf8().append('\0'));
            }
        }

        if (m_ui->tlsPrivateKeyPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPrivateKeyPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->tlsPrivateKeyPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPrivateKeyPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPrivateKeyPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodLeap) {
        if (!m_ui->leapUsername->text().isEmpty()) {
            setting.setIdentity(m_ui->leapUsername->text());
        }

        if (!m_ui->leapPassword->text().isEmpty()) {
            setting.setPassword(m_ui->leapPassword->text());
        }

        if (m_ui->leapPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->leapPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodPwd) {
        if (!m_ui->pwdUsername->text().isEmpty()) {
            setting.setIdentity(m_ui->pwdUsername->text());
        }

        if (m_ui->pwdPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->pwdPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }

        if (!m_ui->pwdPassword->text().isEmpty()) {
            setting.setPassword(m_ui->pwdPassword->text());
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodFast) {
        if (!m_ui->fastAnonIdentity->text().isEmpty()) {
            setting.setAnonymousIdentity(m_ui->fastAnonIdentity->text());
        }

        if (!m_ui->fastAllowPacProvisioning->isChecked()) {
            setting.setPhase1FastProvisioning(NetworkManager::Security8021xSetting::FastProvisioningDisabled);
        } else {
            setting.setPhase1FastProvisioning(static_cast<NetworkManager::Security8021xSetting::FastProvisioning>(m_ui->pacMethod->currentIndex() + 1));
        }

        if (m_ui->pacFile->url().isValid()) {
            setting.setPacFile(QFile::encodeName(m_ui->pacFile->url().toLocalFile()));
        }

        if (m_ui->fastInnerAuth->currentIndex() == 0) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodGtc);
        } else {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodMschapv2);
        }

        if (!m_ui->fastUsername->text().isEmpty()) {
            setting.setIdentity(m_ui->fastUsername->text());
        }

        if (!m_ui->fastPassword->text().isEmpty()) {
            setting.setPassword(m_ui->fastPassword->text());
        }

        if (m_ui->fastPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->fastPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodTtls) {
        if (!m_ui->ttlsAnonIdentity->text().isEmpty()) {
            setting.setAnonymousIdentity(m_ui->ttlsAnonIdentity->text());
        }

        if (!m_ui->ttlsDomain->text().isEmpty()) {
            setting.setDomainSuffixMatch(m_ui->ttlsDomain->text());
        }

        if (m_ui->ttlsCACert->url().isValid()) {
            setting.setCaCertificate(m_ui->ttlsCACert->url().toString().toUtf8().append('\0'));
        }

        const int innerAuth = m_ui->ttlsInnerAuth->currentIndex();
        if (innerAuth == 0) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodPap);
        } else if (innerAuth == 1) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodMschap);
        } else if (innerAuth == 2) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodMschapv2);
        } else if (innerAuth == 3) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodChap);
        }

        if (!m_ui->ttlsUsername->text().isEmpty()) {
            setting.setIdentity(m_ui->ttlsUsername->text());
        }

        if (!m_ui->ttlsPassword->text().isEmpty()) {
            setting.setPassword(m_ui->ttlsPassword->text());
        }

        if (m_ui->ttlsPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->ttlsPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (method == NetworkManager::Security8021xSetting::EapMethodPeap) {
        if (!m_ui->peapAnonIdentity->text().isEmpty()) {
            setting.setAnonymousIdentity(m_ui->peapAnonIdentity->text());
        }

        if (!m_ui->peapDomain->text().isEmpty()) {
            setting.setDomainSuffixMatch(m_ui->peapDomain->text());
        }

        if (m_ui->peapCACert->url().isValid()) {
            setting.setCaCertificate(m_ui->peapCACert->url().toString().toUtf8().append('\0'));
        }

        setting.setPhase1PeapVersion(static_cast<NetworkManager::Security8021xSetting::PeapVersion>(m_ui->peapVersion->currentIndex() - 1));
        const int innerAuth = m_ui->peapInnerAuth->currentIndex();
        if (innerAuth == 0) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodMschapv2);
        } else if (innerAuth == 1) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodMd5);
        } else if (innerAuth == 2) {
            setting.setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethodGtc);
        }

        if (!m_ui->peapUsername->text().isEmpty()) {
            setting.setIdentity(m_ui->peapUsername->text());
        }

        if (!m_ui->peapPassword->text().isEmpty()) {
            setting.setPassword(m_ui->peapPassword->text());
        }

        if (m_ui->peapPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            setting.setPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->peapPassword->passwordOption() == PasswordField::StoreForUser) {
            setting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            setting.setPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    }

    return setting.toMap();
}

void Security8021x::setPasswordOption(PasswordField::PasswordOption option)
{
    m_ui->fastPassword->setPasswordOption(option);
    m_ui->leapPassword->setPasswordOption(option);
    m_ui->md5Password->setPasswordOption(option);
    m_ui->peapPassword->setPasswordOption(option);
    m_ui->pwdPassword->setPasswordOption(option);
    m_ui->tlsPrivateKeyPassword->setPasswordOption(option);
    m_ui->ttlsPassword->setPasswordOption(option);
}

void Security8021x::altSubjectMatchesButtonClicked()
{
    QPointer<EditListDialog> editor = new EditListDialog(this);
    editor->setAttribute(Qt::WA_DeleteOnClose);

    editor->setItems(m_ui->leTlsSubjectMatch->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), Qt::SkipEmptyParts));
    editor->setWindowTitle(i18n("Alternative Subject Matches"));
    editor->setToolTip(
        i18n("<qt>This entry must be one of:<ul><li>DNS: &lt;name or ip address&gt;</li><li>EMAIL: &lt;email&gt;</li><li>URI: &lt;uri, e.g. "
             "https://www.kde.org&gt;</li></ul></qt>"));
    editor->setValidator(altSubjectValidator);

    connect(editor.data(), &QDialog::accepted, [editor, this]() {
        m_ui->leTlsSubjectMatch->setText(editor->items().join(QLatin1String(", ")));
    });
    editor->setModal(true);
    editor->show();
}

void Security8021x::connectToServersButtonClicked()
{
    QPointer<EditListDialog> editor = new EditListDialog(this);
    editor->setAttribute(Qt::WA_DeleteOnClose);

    editor->setItems(m_ui->leTlsConnectToServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), Qt::SkipEmptyParts));
    editor->setWindowTitle(i18n("Connect to these servers only"));
    editor->setValidator(serversValidator);

    connect(editor.data(), &QDialog::accepted, [editor, this]() {
        m_ui->leTlsConnectToServers->setText(editor->items().join(QLatin1String(", ")));
    });
    editor->setModal(true);
    editor->show();
}

bool Security8021x::isValid() const
{
    NetworkManager::Security8021xSetting::EapMethod method =
        static_cast<NetworkManager::Security8021xSetting::EapMethod>(m_ui->auth->itemData(m_ui->auth->currentIndex()).toInt());

    if (method == NetworkManager::Security8021xSetting::EapMethodMd5) {
        return !m_ui->md5UserName->text().isEmpty()
            && (!m_ui->md5Password->text().isEmpty() || m_ui->md5Password->passwordOption() == PasswordField::AlwaysAsk);
    } else if (method == NetworkManager::Security8021xSetting::EapMethodTls) {
        if (m_ui->tlsIdentity->text().isEmpty()) {
            return false;
        }

        if (!m_ui->tlsPrivateKey->url().isValid()) {
            return false;
        }

        if (m_ui->tlsPrivateKeyPassword->passwordOption() == PasswordField::AlwaysAsk) {
            return true;
        }

        if (m_ui->tlsPrivateKeyPassword->text().isEmpty()) {
            return false;
        }

        QCA::Initializer init;
        QCA::ConvertResult convRes;

        // Try if the private key is in pkcs12 format bundled with client certificate
        if (QCA::isSupported("pkcs12")) {
            QCA::KeyBundle keyBundle = QCA::KeyBundle::fromFile(m_ui->tlsPrivateKey->url().path(), m_ui->tlsPrivateKeyPassword->text().toUtf8(), &convRes);
            // We can return the result of decryption when we managed to import the private key
            if (convRes == QCA::ConvertGood) {
                return keyBundle.privateKey().canDecrypt();
            }
        }

        // If the private key is not in pkcs12 format, we need client certificate to be set
        if (!m_ui->tlsUserCert->url().isValid()) {
            return false;
        }

        // Try if the private key is in PEM format and return the result of decryption if we managed to open it
        QCA::PrivateKey key = QCA::PrivateKey::fromPEMFile(m_ui->tlsPrivateKey->url().path(), m_ui->tlsPrivateKeyPassword->text().toUtf8(), &convRes);
        if (convRes == QCA::ConvertGood) {
            return key.canDecrypt();
        }

        // TODO Try other formats (DER - mainly used in Windows)
        // TODO Validate other certificates??
    } else if (method == NetworkManager::Security8021xSetting::EapMethodLeap) {
        return !m_ui->leapUsername->text().isEmpty()
            && (!m_ui->leapPassword->text().isEmpty() || m_ui->leapPassword->passwordOption() == PasswordField::AlwaysAsk);
    } else if (method == NetworkManager::Security8021xSetting::EapMethodPwd) {
        return !m_ui->pwdUsername->text().isEmpty()
            && (!m_ui->pwdPassword->text().isEmpty() || m_ui->pwdPassword->passwordOption() == PasswordField::AlwaysAsk);
    } else if (method == NetworkManager::Security8021xSetting::EapMethodFast) {
        if (!m_ui->fastAllowPacProvisioning->isChecked() && !m_ui->pacFile->url().isValid()) {
            return false;
        }
        return !m_ui->fastUsername->text().isEmpty()
            && (!m_ui->fastPassword->text().isEmpty() || m_ui->fastPassword->passwordOption() == PasswordField::AlwaysAsk);
    } else if (method == NetworkManager::Security8021xSetting::EapMethodTtls) {
        return !m_ui->ttlsUsername->text().isEmpty()
            && (!m_ui->ttlsPassword->text().isEmpty() || m_ui->ttlsPassword->passwordOption() == PasswordField::AlwaysAsk);
    } else if (method == NetworkManager::Security8021xSetting::EapMethodPeap) {
        return !m_ui->peapUsername->text().isEmpty()
            && (!m_ui->peapPassword->text().isEmpty() || m_ui->peapPassword->passwordOption() == PasswordField::AlwaysAsk);
    }

    return true;
}

void Security8021x::currentAuthChanged(int index)
{
    Q_UNUSED(index);
    KAcceleratorManager::manage(m_ui->stackedWidget->currentWidget());
}

#include "moc_security802-1x.cpp"
