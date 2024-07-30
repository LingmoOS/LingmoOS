/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultimportingwizard.h"
#include "ui_vaultimportingwizard.h"

#include <QList>
#include <QMap>
#include <QPushButton>
#include <QStackedLayout>

#include "dialogdsl.h"
#include "vault.h"

using namespace DialogDsl;
using namespace DialogDsl::operators;

#include "activitieslinkingwidget.h"
#include "backendchooserwidget.h"
#include "cryfscypherchooserwidget.h"
#include "directorypairchooserwidget.h"
#include "noticewidget.h"
#include "offlineonlywidget.h"
#include "passwordchooserwidget.h"

#include "vaultwizardbase.h"

class VaultImportingWizard::Private : public WBASE(VaultImportingWizard)
{
public:
    Logic logic{{"encfs" / i18n("EncFS"),
                 {step{directoryPairChooser(DirectoryPairChooserWidget::ShowDevicePicker | DirectoryPairChooserWidget::ShowMountPointPicker
                                            | DirectoryPairChooserWidget::RequireExistingDevice | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{passwordChooser()},
                  step{activitiesChooser(), offlineOnlyChooser()}}},

                {"cryfs" / i18n("CryFS"),
                 {step{directoryPairChooser(DirectoryPairChooserWidget::ShowDevicePicker | DirectoryPairChooserWidget::ShowMountPointPicker
                                            | DirectoryPairChooserWidget::RequireExistingDevice | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{passwordChooser()},
                  step{activitiesChooser(), offlineOnlyChooser()}}},

                {"gocryptfs" / i18n("gocryptfs"),
                 {step{directoryPairChooser(DirectoryPairChooserWidget::ShowDevicePicker | DirectoryPairChooserWidget::ShowMountPointPicker
                                            | DirectoryPairChooserWidget::RequireExistingDevice | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{passwordChooser()},
                  step{activitiesChooser(), offlineOnlyChooser()}}}};

    Private(VaultImportingWizard *parent)
        : WBASE(VaultImportingWizard)(parent)
    {
        lastButtonText = i18n("Import");
        initBase();
    }

    void finish()
    {
        auto collectedPayload = firstStepModule->fields();
        for (const auto *module : currentStepModules) {
            collectedPayload.insert(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::Device device(collectedPayload[KEY_DEVICE].toString());
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());

        auto vault = new PlasmaVault::Vault(device, q);

        auto future = vault->import(name, mountPoint, collectedPayload);

        auto result = AsynQt::await(future);

        if (result) {
            Q_EMIT q->importedVault(vault);
            q->QDialog::accept();

        } else {
            ui.message->setText(result.error().message());
            ui.message->setMessageType(KMessageWidget::Error);
            ui.message->show();
            vault->scheduleDeletion();
        }
    }
};

VaultImportingWizard::VaultImportingWizard(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
    setWindowTitle(i18nc("@title:window", "Import an Existing Vault"));
}

VaultImportingWizard::~VaultImportingWizard()
{
}

#include "moc_vaultimportingwizard.cpp"
