/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iodineauth.h"
#include "ui_iodineauth.h"

#include "nm-iodine-service.h"

#include <QString>

class IodineAuthDialogPrivate
{
public:
    Ui_IodineAuth ui;
    NetworkManager::VpnSetting::Ptr setting;
};

IodineAuthDialog::IodineAuthDialog(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new IodineAuthDialogPrivate)
{
    Q_D(IodineAuthDialog);
    d->ui.setupUi(this);
    d->setting = setting;

    KAcceleratorManager::manage(this);
}

IodineAuthDialog::~IodineAuthDialog()
{
    delete d_ptr;
}

QVariantMap IodineAuthDialog::setting() const
{
    Q_D(const IodineAuthDialog);

    NMStringMap secrets;
    QVariantMap secretData;

    if (!d->ui.le_password->text().isEmpty()) {
        secrets.insert(QLatin1String(NM_IODINE_KEY_PASSWORD), d->ui.le_password->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_iodineauth.cpp"
