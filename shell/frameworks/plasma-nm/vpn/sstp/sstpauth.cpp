/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sstpauth.h"
#include "ui_sstpauth.h"

#include "nm-sstp-service.h"

class SstpAuthWidgetPrivate
{
public:
    NetworkManager::VpnSetting::Ptr setting;
    Ui_SstpAuth ui;
};

SstpAuthWidget::SstpAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new SstpAuthWidgetPrivate)
{
    Q_D(SstpAuthWidget);
    d->setting = setting;
    d->ui.setupUi(this);

    KAcceleratorManager::manage(this);
}

SstpAuthWidget::~SstpAuthWidget()
{
    delete d_ptr;
}

QVariantMap SstpAuthWidget::setting() const
{
    Q_D(const SstpAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;

    if (!d->ui.le_password->text().isEmpty()) {
        secrets.insert(QLatin1String(NM_SSTP_KEY_PASSWORD), d->ui.le_password->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_sstpauth.cpp"
