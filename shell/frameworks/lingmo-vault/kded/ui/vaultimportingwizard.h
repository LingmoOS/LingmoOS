/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef LINGMOVAULT_KDED_UI_VAULT_IMPORTING_WIZARD_H
#define LINGMOVAULT_KDED_UI_VAULT_IMPORTING_WIZARD_H

#include <QDialog>

namespace LingmoVault
{
class Vault;
} // namespace LingmoVault

class VaultImportingWizard : public QDialog
{
    Q_OBJECT

public:
    VaultImportingWizard(QWidget *parent = nullptr);
    ~VaultImportingWizard() override;

Q_SIGNALS:
    void importedVault(LingmoVault::Vault *vault);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // include guard
