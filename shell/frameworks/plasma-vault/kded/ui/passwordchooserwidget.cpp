/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "passwordchooserwidget.h"

#include "ui_passwordchooserwidget.h"

class PasswordChooserWidget::Private
{
public:
    Ui::PasswordChooserWidget ui;
};

PasswordChooserWidget::PasswordChooserWidget()
    : DialogDsl::DialogModule(false)
    , d(new Private())
{
    d->ui.setupUi(this);

    connect(d->ui.editPassword, &KNewPasswordWidget::passwordStatusChanged, this, [&] {
        const auto status = d->ui.editPassword->passwordStatus();
        setIsValid(status == KNewPasswordWidget::StrongPassword || status == KNewPasswordWidget::WeakPassword);
    });
}

PasswordChooserWidget::~PasswordChooserWidget()
{
}

PlasmaVault::Vault::Payload PasswordChooserWidget::fields() const
{
    return {{KEY_PASSWORD, d->ui.editPassword->password()}};
}

#include "moc_passwordchooserwidget.cpp"
