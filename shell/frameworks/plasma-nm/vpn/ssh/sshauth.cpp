/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sshauth.h"
#include "ui_sshauth.h"

#include "nm-ssh-service.h"

class SshAuthWidgetPrivate
{
public:
    NetworkManager::VpnSetting::Ptr setting;
    Ui_SshAuth ui;
};

SshAuthWidget::SshAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new SshAuthWidgetPrivate)
{
    Q_D(SshAuthWidget);
    d->setting = setting;
    d->ui.setupUi(this);

    KAcceleratorManager::manage(this);
}

SshAuthWidget::~SshAuthWidget()
{
    delete d_ptr;
}

QVariantMap SshAuthWidget::setting() const
{
    Q_D(const SshAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;

    if (!d->ui.le_password->text().isEmpty()) {
        secrets.insert(QLatin1String(NM_SSH_KEY_PASSWORD), d->ui.le_password->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_sshauth.cpp"
