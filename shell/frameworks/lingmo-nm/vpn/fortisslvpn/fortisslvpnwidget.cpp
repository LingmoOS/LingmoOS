/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fortisslvpnwidget.h"
#include "ui_fortisslvpn.h"
#include "ui_fortisslvpnadvanced.h"

#include "nm-fortisslvpn-service.h"

#include <NetworkManagerQt/Setting>

#include <QDialog>
#include <QDialogButtonBox>

class FortisslvpnWidgetPrivate
{
public:
    Ui::FortisslvpnWidget ui;
    Ui::FortisslvpnAdvancedWidget advUi;
    NetworkManager::VpnSetting::Ptr setting;
    QDialog *advancedDlg = nullptr;
    QWidget *advancedWid = nullptr;
};

FortisslvpnWidget::FortisslvpnWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , d_ptr(new FortisslvpnWidgetPrivate)
{
    Q_D(FortisslvpnWidget);

    d->setting = setting;

    d->ui.setupUi(this);

    d->ui.password->setPasswordOptionsEnabled(true);
    d->ui.password->setPasswordNotRequiredEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(d->ui.gateway, &QLineEdit::textChanged, this, &FortisslvpnWidget::slotWidgetChanged);

    // Advanced configuration
    connect(d->ui.advancedButton, &QPushButton::clicked, this, &FortisslvpnWidget::showAdvanced);

    d->advancedDlg = new QDialog(this);
    d->advancedWid = new QWidget(this);
    d->advUi.setupUi(d->advancedWid);
    auto layout = new QVBoxLayout(d->advancedDlg);
    layout->addWidget(d->advancedWid);
    d->advancedDlg->setLayout(layout);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, d->advancedDlg);
    connect(buttons, &QDialogButtonBox::accepted, d->advancedDlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, d->advancedDlg, &QDialog::reject);
    layout->addWidget(buttons);
    KAcceleratorManager::manage(this);

    // Remove these from setting check:
    // Just popping up the advancedDlg changes nothing
    disconnect(d->ui.advancedButton, &QPushButton::clicked, this, &SettingWidget::settingChanged);
    // But the accept button does
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingWidget::settingChanged);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

FortisslvpnWidget::~FortisslvpnWidget()
{
    delete d_ptr;
}

void FortisslvpnWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(FortisslvpnWidget);

    const NMStringMap data = d->setting->data();

    const QString gateway = data.value(NM_FORTISSLVPN_KEY_GATEWAY);
    if (!gateway.isEmpty()) {
        d->ui.gateway->setText(gateway);
    }

    const QString username = data.value(NM_FORTISSLVPN_KEY_USER);
    if (!username.isEmpty()) {
        d->ui.username->setText(username);
    }

    const NetworkManager::Setting::SecretFlags passwordFlag =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_PASSWORD "-flags").toInt());
    if (passwordFlag == NetworkManager::Setting::None) {
        d->ui.password->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (passwordFlag == NetworkManager::Setting::AgentOwned) {
        d->ui.password->setPasswordOption(PasswordField::StoreForUser);
    } else if (passwordFlag == NetworkManager::Setting::NotSaved) {
        d->ui.password->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        d->ui.password->setPasswordOption(PasswordField::NotRequired);
    }

    const QString caCert = data.value(NM_FORTISSLVPN_KEY_CA);
    if (!caCert.isEmpty()) {
        d->ui.caCert->setText(caCert);
    }

    const QString userCert = data.value(NM_FORTISSLVPN_KEY_CERT);
    if (!userCert.isEmpty()) {
        d->ui.userCert->setText(userCert);
    }

    const QString userKey = data.value(NM_FORTISSLVPN_KEY_KEY);
    if (!userKey.isEmpty()) {
        d->ui.userKey->setText(userKey);
    }

    // From advanced dialog
    const QString trustedCert = data.value(NM_FORTISSLVPN_KEY_TRUSTED_CERT);
    if (!trustedCert.isEmpty()) {
        d->advUi.trustedCert->setText(trustedCert);
    }

    if (!data.value(NM_FORTISSLVPN_KEY_OTP "-flags").isEmpty()) {
        const NetworkManager::Setting::SecretFlags otpFlag =
            static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_OTP "-flags").toInt());
        if (otpFlag & NetworkManager::Setting::NotSaved) {
            d->advUi.otp->setChecked(true);
        }
    }

    if (!data.value(NM_FORTISSLVPN_KEY_2FA "-flags").isEmpty()) {
        const NetworkManager::Setting::SecretFlags tfaFlag =
            static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_2FA "-flags").toInt());
        if (tfaFlag & NetworkManager::Setting::AgentOwned) {
            d->advUi.tfa->setChecked(true);
        }
    }

    const QString realm = data.value(NM_FORTISSLVPN_KEY_REALM);
    if (!realm.isEmpty()) {
        d->advUi.realm->setText(realm);
    }

    loadSecrets(setting);
}

void FortisslvpnWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(FortisslvpnWidget);

    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();

        const QString password = secrets.value(NM_FORTISSLVPN_KEY_PASSWORD);
        if (!password.isEmpty()) {
            d->ui.password->setText(password);
        }
    }
}

QVariantMap FortisslvpnWidget::setting() const
{
    Q_D(const FortisslvpnWidget);

    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_FORTISSLVPN));
    NMStringMap data;
    NMStringMap secrets;

    data.insert(NM_FORTISSLVPN_KEY_GATEWAY, d->ui.gateway->text());

    if (!d->ui.username->text().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_USER, d->ui.username->text());
    }

    if (!d->ui.password->text().isEmpty()) {
        secrets.insert(NM_FORTISSLVPN_KEY_PASSWORD, d->ui.password->text());
    }

    if (d->ui.password->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_FORTISSLVPN_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::None));
    } else if (d->ui.password->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_FORTISSLVPN_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else if (d->ui.password->passwordOption() == PasswordField::AlwaysAsk) {
        data.insert(NM_FORTISSLVPN_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotSaved));
    } else {
        data.insert(NM_FORTISSLVPN_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotRequired));
    }

    if (!d->ui.caCert->url().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_CA, d->ui.caCert->url().toLocalFile());
    }

    if (!d->ui.userCert->url().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_CERT, d->ui.userCert->url().toLocalFile());
    }

    if (!d->ui.userKey->url().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_KEY, d->ui.userKey->url().toLocalFile());
    }

    // From advanced
    if (!d->advUi.trustedCert->text().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_TRUSTED_CERT, d->advUi.trustedCert->text());
    }

    if (d->advUi.otp->isChecked()) {
        data.insert(QLatin1String(NM_FORTISSLVPN_KEY_OTP "-flags"), QString::number(NetworkManager::Setting::NotSaved));
    } else {
        data.insert(QLatin1String(NM_FORTISSLVPN_KEY_OTP "-flags"), QString::number(NetworkManager::Setting::None));
    }

    if (d->advUi.tfa->isChecked()) {
        data.insert(QLatin1String(NM_FORTISSLVPN_KEY_2FA "-flags"), QString::number(NetworkManager::Setting::AgentOwned));
        data.insert(QLatin1String(NM_FORTISSLVPN_KEY_OTP "-flags"), QString::number(NetworkManager::Setting::None));
    }

    if (!d->advUi.realm->text().isEmpty()) {
        data.insert(NM_FORTISSLVPN_KEY_REALM, d->advUi.realm->text());
    }

    setting.setData(data);
    setting.setSecrets(secrets);

    return setting.toMap();
}

void FortisslvpnWidget::showAdvanced()
{
    Q_D(FortisslvpnWidget);

    d->advancedDlg->show();
}

bool FortisslvpnWidget::isValid() const
{
    Q_D(const FortisslvpnWidget);

    return !d->ui.gateway->text().isEmpty();
}

#include "moc_fortisslvpnwidget.cpp"
