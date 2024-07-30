/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_NAME_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_NAME_CHOOSER_WIDGET_H

#include "dialogdsl.h"

class NameChooserWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    NameChooserWidget();
    ~NameChooserWidget() override;

    PlasmaVault::Vault::Payload fields() const override;
    void init(const PlasmaVault::Vault::Payload &payload) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory nameChooser()
{
    return [=] {
        return new NameChooserWidget();
    };
}

#endif // include guard
