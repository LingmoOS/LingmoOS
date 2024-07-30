/*
    SPDX-FileCopyrightText: 2010 Andrey Borzenkov <arvidjaar@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "vpncauth.h"
#include "nm-vpnc-service.h"
#include "ui_vpncauth.h"

#include <QString>

class VpncAuthDialogPrivate
{
public:
    Ui_VpncAuth ui;
    NetworkManager::VpnSetting::Ptr setting;
};

VpncAuthDialog::VpncAuthDialog(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new VpncAuthDialogPrivate)
{
    Q_D(VpncAuthDialog);
    d->ui.setupUi(this);
    d->setting = setting;

    readSecrets();

    KAcceleratorManager::manage(this);
}

VpncAuthDialog::~VpncAuthDialog()
{
    delete d_ptr;
}

void VpncAuthDialog::readSecrets()
{
    Q_D(VpncAuthDialog);
    const NMStringMap data = d->setting->data();
    const NMStringMap secrets = d->setting->secrets();

    //   username
    const QString user = data.value(NM_VPNC_KEY_XAUTH_USER);
    if (!user.isEmpty()) {
        d->ui.leUserName->setText(user);
    }
    //   group name
    const QString group = data.value(NM_VPNC_KEY_ID);
    if (!group.isEmpty()) {
        d->ui.leGroupName->setText(group);
    }

    bool haveUserPassword = true;
    if (!((NetworkManager::Setting::SecretFlags)data.value(NM_VPNC_KEY_XAUTH_PASSWORD "-flags").toInt()).testFlag(NetworkManager::Setting::NotRequired)) {
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_XAUTH_PASSWORD)));
    } else {
        d->ui.userNameLabel->setVisible(false);
        d->ui.leUserName->setVisible(false);
        d->ui.userPasswordLabel->setVisible(false);
        d->ui.leUserPassword->setVisible(false);
        haveUserPassword = false;
    }

    if (!((NetworkManager::Setting::SecretFlags)data.value(NM_VPNC_KEY_SECRET "-flags").toInt()).testFlag(NetworkManager::Setting::NotRequired)) {
        d->ui.leGroupPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_SECRET)));
    } else {
        d->ui.groupNameLabel->setVisible(false);
        d->ui.leGroupName->setVisible(false);
        d->ui.groupPasswordLabel->setVisible(false);
        d->ui.leGroupPassword->setVisible(false);
    }

    if (haveUserPassword && d->ui.leUserPassword->text().isEmpty()) {
        d->ui.leUserPassword->setFocus(Qt::OtherFocusReason);
    } else if (d->ui.leGroupPassword->text().isEmpty()) {
        d->ui.leGroupPassword->setFocus(Qt::OtherFocusReason);
    }
}

QVariantMap VpncAuthDialog::setting() const
{
    Q_D(const VpncAuthDialog);

    NMStringMap secrets;
    QVariantMap result;

    //   user password
    if (!d->ui.leUserPassword->text().isEmpty()) {
        secrets.insert(NM_VPNC_KEY_XAUTH_PASSWORD, d->ui.leUserPassword->text());
    }
    //   group password
    if (!d->ui.leGroupPassword->text().isEmpty()) {
        secrets.insert(NM_VPNC_KEY_SECRET, d->ui.leGroupPassword->text());
    }

    result.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));

    return result;
}

#include "moc_vpncauth.cpp"
