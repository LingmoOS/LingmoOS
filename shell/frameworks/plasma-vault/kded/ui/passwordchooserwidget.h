/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_PASSWORD_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_PASSWORD_CHOOSER_WIDGET_H

#include "dialogdsl.h"

class PasswordChooserWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    PasswordChooserWidget();
    ~PasswordChooserWidget() override;

    PlasmaVault::Vault::Payload fields() const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory passwordChooser()
{
    return [=] {
        return new PasswordChooserWidget();
    };
}

#endif // include guard
