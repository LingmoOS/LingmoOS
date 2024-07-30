/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "namechooserwidget.h"
#include "ui_namechooserwidget.h"

#include "vault.h"

class NameChooserWidget::Private
{
public:
    Ui::NameChooserWidget ui;

    NameChooserWidget *const q;
    Private(NameChooserWidget *parent)
        : q(parent)
    {
    }
};

NameChooserWidget::NameChooserWidget()
    : DialogDsl::DialogModule(false)
    , d(new Private(this))
{
    d->ui.setupUi(this);

    connect(d->ui.editVaultName, &QLineEdit::textChanged, this, [this](const QString &text) {
        Q_UNUSED(text);
        setIsValid(!d->ui.editVaultName->text().isEmpty());
    });
}

NameChooserWidget::~NameChooserWidget()
{
}

PlasmaVault::Vault::Payload NameChooserWidget::fields() const
{
    return {{KEY_NAME, d->ui.editVaultName->text()}};
}

void NameChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    const auto name = payload[KEY_NAME].toString();

    d->ui.editVaultName->setText(name);
    setIsValid(!d->ui.editVaultName->text().isEmpty());
}

#include "moc_namechooserwidget.cpp"
