/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_DIRECTORY_PAIR_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_DIRECTORY_PAIR_CHOOSER_WIDGET_H

#include "dialogdsl.h"

class DirectoryPairChooserWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    enum Flag {
        ShowDevicePicker = 1,
        ShowMountPointPicker = 2,
        RequireEmptyDevice = 4,
        RequireExistingDevice = 8,
        RequireEmptyMountPoint = 16,
        RequireExistingMountPoint = 32,

        AutoFillPaths = 64,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    DirectoryPairChooserWidget(Flags flags);
    ~DirectoryPairChooserWidget() override;

    PlasmaVault::Vault::Payload fields() const override;
    void init(const PlasmaVault::Vault::Payload &payload) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory directoryPairChooser(DirectoryPairChooserWidget::Flags flags)
{
    return [=] {
        return new DirectoryPairChooserWidget(flags);
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(DirectoryPairChooserWidget::Flags)

#endif // include guard
