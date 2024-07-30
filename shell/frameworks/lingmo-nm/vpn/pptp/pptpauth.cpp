/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pptpauth.h"
#include "ui_pptpauth.h"

#include "nm-pptp-service.h"

class PptpAuthWidgetPrivate
{
public:
    NetworkManager::VpnSetting::Ptr setting;
    Ui_PptpAuthenticationWidget ui;
};

PptpAuthWidget::PptpAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new PptpAuthWidgetPrivate)
{
    Q_D(PptpAuthWidget);
    d->setting = setting;
    d->ui.setupUi(this);

    KAcceleratorManager::manage(this);
}

PptpAuthWidget::~PptpAuthWidget()
{
    delete d_ptr;
}

QVariantMap PptpAuthWidget::setting() const
{
    Q_D(const PptpAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;

    if (!d->ui.lePassword->text().isEmpty()) {
        secrets.insert(QLatin1String(NM_PPTP_KEY_PASSWORD), d->ui.lePassword->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_pptpauth.cpp"
