/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_VAULT_CREATION_WIZARD_H
#define PLASMAVAULT_KDED_UI_VAULT_CREATION_WIZARD_H

#include <QDialog>

namespace PlasmaVault
{
class Vault;
} // namespace PlasmaVault

class VaultCreationWizard : public QDialog
{
    Q_OBJECT

public:
    VaultCreationWizard(QWidget *parent = nullptr);
    ~VaultCreationWizard() override;

Q_SIGNALS:
    void createdVault(PlasmaVault::Vault *vault);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // include guard
