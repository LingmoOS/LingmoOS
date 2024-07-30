/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Maurus Rohrer <maurus.rohrer@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "strongswanwidget.h"
#include "nm-strongswan-service.h"
#include "ui_strongswanprop.h"

#include <QString>
#include <QUrl>

class StrongswanSettingWidgetPrivate
{
public:
    Ui_StrongswanProp ui;
    NetworkManager::VpnSetting::Ptr setting;
    enum AuthType { PrivateKey = 0, SshAgent, Smartcard, Eap };
};

StrongswanSettingWidget::StrongswanSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , d_ptr(new StrongswanSettingWidgetPrivate)
{
    Q_D(StrongswanSettingWidget);
    d->ui.setupUi(this);

    d->setting = setting;
    
    // Enables combo box for password
    d->ui.leUserPassword->setPasswordOptionsEnabled(true);
    d->ui.leUserPassword->setPasswordNotRequiredEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(d->ui.leGateway, &QLineEdit::textChanged, this, &StrongswanSettingWidget::slotWidgetChanged);
    connect(d->ui.proposal, &QGroupBox::toggled, this, &SettingWidget::settingChanged);

    KAcceleratorManager::manage(this);

    if (d->setting && !d->setting->isNull()) {
        loadConfig(d->setting);
    }
}

StrongswanSettingWidget::~StrongswanSettingWidget()
{
    delete d_ptr;
}

void StrongswanSettingWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting)
    Q_D(StrongswanSettingWidget);

    // General settings
    const NMStringMap dataMap = d->setting->data();
    // Gateway Address
    const QString gateway = dataMap[NM_STRONGSWAN_GATEWAY];
    if (!gateway.isEmpty()) {
        d->ui.leGateway->setText(gateway);
    }
    // Certificate
    d->ui.leGatewayCertificate->setUrl(QUrl::fromLocalFile(dataMap[NM_STRONGSWAN_CERTIFICATE]));
    
    // Remote Identity
    const QString rIdentity = dataMap[NM_STRONGSWAN_RIDENTITY];
    if (!rIdentity.isEmpty()) {
        d->ui.leRemoteIdentity->setText(rIdentity);
    }

    // Authentication
    const QString method = dataMap[NM_STRONGSWAN_METHOD];
    if (method == QLatin1String(NM_STRONGSWAN_AUTH_KEY)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::PrivateKey);
        d->ui.leAuthPrivatekeyCertificate->setUrl(QUrl::fromLocalFile(dataMap[NM_STRONGSWAN_USERCERT]));
        d->ui.leAuthPrivatekeyKey->setUrl(QUrl::fromLocalFile(dataMap[NM_STRONGSWAN_USERKEY]));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_AGENT)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::SshAgent);
        d->ui.leAuthSshCertificate->setUrl(QUrl::fromLocalFile(dataMap[NM_STRONGSWAN_USERCERT]));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_SMARTCARD)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::Smartcard);
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_EAP)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::Eap);
        d->ui.leUserName->setText(dataMap[NM_STRONGSWAN_USER]);
        
        PasswordField *passwordField = d->ui.leUserPassword;
        const NetworkManager::Setting::SecretFlags type = (NetworkManager::Setting::SecretFlags)dataMap[NM_STRONGSWAN_POPTION].toInt();
        if (type.testFlag(NetworkManager::Setting::None)) {
            passwordField->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (type.testFlag(NetworkManager::Setting::AgentOwned)) {
            passwordField->setPasswordOption(PasswordField::StoreForUser);
        } else if (type.testFlag(NetworkManager::Setting::NotSaved)) {
            passwordField->setPasswordOption(PasswordField::AlwaysAsk);
        } else if (type.testFlag(NetworkManager::Setting::NotRequired)) {
            passwordField->setPasswordOption(PasswordField::NotRequired);
        }
        // Strongswan always reads the password inside [vpn-secrets] section for global users
        NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();
        const NMStringMap secrets = vpnSetting->secrets();
        d->ui.leUserPassword->setText(secrets.value(NM_STRONGSWAN_SECRET));
    }

    // Settings
    d->ui.innerIP->setChecked(dataMap[NM_STRONGSWAN_INNERIP] == "yes");
    d->ui.udpEncap->setChecked(dataMap[NM_STRONGSWAN_ENCAP] == "yes");
    d->ui.ipComp->setChecked(dataMap[NM_STRONGSWAN_IPCOMP] == "yes");
    d->ui.proposal->setChecked(dataMap[NM_STRONGSWAN_PROPOSAL] == "yes");
    d->ui.ike->setText(dataMap[NM_STRONGSWAN_IKE]);
    d->ui.esp->setText(dataMap[NM_STRONGSWAN_ESP]);
}

void StrongswanSettingWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(StrongswanSettingWidget);
    Q_UNUSED(setting);
}

QVariantMap StrongswanSettingWidget::setting() const
{
    Q_D(const StrongswanSettingWidget);

    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_STRONGSWAN));

    NMStringMap data;
    NMStringMap secretData;

    // General settings
    // Gateway
    if (!d->ui.leGateway->text().isEmpty()) {
        data.insert(NM_STRONGSWAN_GATEWAY, d->ui.leGateway->text());
    }

    const QString certificate = d->ui.leGatewayCertificate->url().toLocalFile();
    if (!certificate.isEmpty()) {
        data.insert(NM_STRONGSWAN_CERTIFICATE, certificate);
    }
        
    // Server Identity (Right Identity)
    if (!d->ui.leRemoteIdentity->text().isEmpty()) {
        data.insert(NM_STRONGSWAN_RIDENTITY, d->ui.leRemoteIdentity->text());
    }

    // Authentication
    switch (d->ui.cmbMethod->currentIndex()) {
    case StrongswanSettingWidgetPrivate::PrivateKey: {
        data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_KEY);
        const QString userPrivateCertificate = d->ui.leAuthPrivatekeyCertificate->url().toLocalFile();
        if (!userPrivateCertificate.isEmpty()) {
            data.insert(NM_STRONGSWAN_USERCERT, userPrivateCertificate);
        }
        const QString userKey = d->ui.leAuthPrivatekeyKey->url().toLocalFile();
        if (!userKey.isEmpty()) {
            data.insert(NM_STRONGSWAN_USERKEY, userKey);
        }
        break;
    }
    case StrongswanSettingWidgetPrivate::SshAgent: {
        data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_AGENT);
        const QString userSshCertificate = d->ui.leAuthSshCertificate->url().toLocalFile();
        if (!userSshCertificate.isEmpty()) {
            data.insert(NM_STRONGSWAN_USERCERT, userSshCertificate);
        }
        break;
    }
    case StrongswanSettingWidgetPrivate::Smartcard:
        data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_SMARTCARD);
        break;
    case StrongswanSettingWidgetPrivate::Eap:
        data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_EAP);
        if (!d->ui.leUserName->text().isEmpty()) {
            data.insert(NM_STRONGSWAN_USER, d->ui.leUserName->text());
        }
        // StrongSwan-nm 1.2 does not appear to be able to save secrets, the must be entered through the auth dialog
        // flags for password
        PasswordField *passwordField = d->ui.leUserPassword;
        const PasswordField::PasswordOption option = passwordField->passwordOption();
        if (option == PasswordField::StoreForAllUsers) {
            data.insert(NM_STRONGSWAN_POPTION, QString::number(NetworkManager::Setting::None));
        } else if (option == PasswordField::StoreForUser) {
            data.insert(NM_STRONGSWAN_POPTION, QString::number(NetworkManager::Setting::AgentOwned));
        } else if (option == PasswordField::AlwaysAsk) {
            data.insert(NM_STRONGSWAN_POPTION, QString::number(NetworkManager::Setting::NotSaved));
        } else {
            data.insert(NM_STRONGSWAN_POPTION, QString::number(NetworkManager::Setting::NotRequired));
        }
        // Saving the password
        if (!d->ui.leUserPassword->text().isEmpty()) {
            secretData = d->setting->secrets();
            secretData.insert(NM_STRONGSWAN_SECRET, d->ui.leUserPassword->text());
        }
    }

    // Options
    data.insert(NM_STRONGSWAN_INNERIP, d->ui.innerIP->isChecked() ? "yes" : "no");
    data.insert(NM_STRONGSWAN_ENCAP, d->ui.udpEncap->isChecked() ? "yes" : "no");
    data.insert(NM_STRONGSWAN_IPCOMP, d->ui.ipComp->isChecked() ? "yes" : "no");
    if (d->ui.proposal->isChecked()) {
        data.insert(NM_STRONGSWAN_PROPOSAL, "yes");
        data.insert(NM_STRONGSWAN_IKE, d->ui.ike->text());
        data.insert(NM_STRONGSWAN_ESP, d->ui.esp->text());
    } else
        data.insert(NM_STRONGSWAN_PROPOSAL, "no");

    // save it all
    setting.setData(data);
    setting.setSecrets(secretData);

    return setting.toMap();
}

bool StrongswanSettingWidget::isValid() const
{
    Q_D(const StrongswanSettingWidget);
    return !d->ui.leGateway->text().isEmpty();
}

#include "moc_strongswanwidget.cpp"
