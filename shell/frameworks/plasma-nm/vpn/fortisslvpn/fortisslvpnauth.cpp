/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fortisslvpnauth.h"
#include "ui_fortisslvpnauth.h"

#include "nm-fortisslvpn-service.h"

#include <QString>

class FortisslvpnAuthDialogPrivate
{
public:
    Ui_FortisslvpnAuth ui;
    NetworkManager::VpnSetting::Ptr setting;
};

FortisslvpnAuthDialog::FortisslvpnAuthDialog(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new FortisslvpnAuthDialogPrivate)
{
    Q_D(FortisslvpnAuthDialog);

    d->ui.setupUi(this);
    d->setting = setting;

    const NMStringMap data = d->setting->data();

    const NetworkManager::Setting::SecretFlags otpFlag = static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_OTP "-flags").toInt());
    d->ui.otpFrame->setVisible(otpFlag == NetworkManager::Setting::NotSaved);

    const NetworkManager::Setting::SecretFlags passwordFlag =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_PASSWORD "-flags").toInt());
    d->ui.passwordFrame->setVisible(passwordFlag == NetworkManager::Setting::NotSaved);

    if (m_hints.count() == 2) {
        const QString hint = m_hints.at(0);
        const QString hintMsg = m_hints.at(1);
        d->ui.otpLabel->setText(hint);
        d->ui.labelOtp->setText(hintMsg.section(":", -2));
        d->ui.otpFrame->setVisible(true);
        d->ui.passwordFrame->setVisible(false);
    }

    KAcceleratorManager::manage(this);
}

FortisslvpnAuthDialog::~FortisslvpnAuthDialog()
{
    delete d_ptr;
}

QVariantMap FortisslvpnAuthDialog::setting() const
{
    Q_D(const FortisslvpnAuthDialog);

    const NMStringMap data = d->setting->data();
    NMStringMap secrets;
    QVariantMap secretData;

    if (!d->ui.password->text().isEmpty()) {
        secrets.insert(QLatin1String(NM_FORTISSLVPN_KEY_PASSWORD), d->ui.password->text());
    }

    if (!data.value(NM_FORTISSLVPN_KEY_OTP "-flags").isEmpty()) {
        const NetworkManager::Setting::SecretFlags otpFlag =
            static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_FORTISSLVPN_KEY_OTP "-flags").toInt());
        if (otpFlag == NetworkManager::Setting::NotSaved && !d->ui.otp->text().isEmpty()) {
            secrets.insert(QLatin1String(NM_FORTISSLVPN_KEY_OTP), d->ui.otp->text());
        }
    }

    if (!data.value(NM_FORTISSLVPN_KEY_2FA "-flags").isEmpty()) {
        secrets.insert(QLatin1String(NM_FORTISSLVPN_KEY_2FA), d->ui.otp->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_fortisslvpnauth.cpp"
