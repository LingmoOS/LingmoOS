/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_VAULT_CONFIGURATION_DIALOG_H
#define PLASMAVAULT_KDED_UI_VAULT_CONFIGURATION_DIALOG_H

#include <QDialog>

namespace PlasmaVault
{
class Vault;
} // namespace PlasmaVault

class VaultConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    VaultConfigurationDialog(PlasmaVault::Vault *vault, QWidget *parent = nullptr);
    ~VaultConfigurationDialog() override;

Q_SIGNALS:
    void configurationChanged(PlasmaVault::Vault *vault);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // include guard
