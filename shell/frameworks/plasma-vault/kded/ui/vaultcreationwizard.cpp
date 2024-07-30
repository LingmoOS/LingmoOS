/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultcreationwizard.h"
#include "ui_vaultcreationwizard.h"

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

class VaultCreationWizard::Private : public WBASE(VaultCreationWizard)
{
public:
    Logic logic{{"encfs" / i18n("EncFS"),
                 {step{notice("encfs-message", i18n("<b>Security notice:</b>\n\
                             According to a security audit by Taylor Hornby (Defuse Security),\n\
                             the current implementation of Encfs is vulnerable or potentially vulnerable\n\
                             to multiple types of attacks.\n\
                             For example, an attacker with read/write access\n\
                             to encrypted data might lower the decryption complexity\n\
                             for subsequently encrypted data without this being noticed by a legitimate user,\n\
                             or might use timing analysis to deduce information.\n\
                             <br /><br />\n\
                             This means that you should not synchronize\n\
                             the encrypted data to a cloud storage service,\n\
                             or use it in other circumstances where the attacker\n\
                             can frequently access the encrypted data.\n\
                             <br /><br />\n\
                             See <a href='https://defuse.ca/audits/encfs.htm'>defuse.ca/audits/encfs.htm</a> for more information."))},
                  step{passwordChooser()},
                  step{directoryPairChooser(DirectoryPairChooserWidget::AutoFillPaths | DirectoryPairChooserWidget::ShowMountPointPicker
                                            | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{activitiesChooser(), offlineOnlyChooser()}}},

                {"cryfs" / i18n("CryFS"),
                 {step{notice("cryfs-message", i18n("<b>Security notice:</b>\n\
                             CryFS encrypts your files, so you can safely store them anywhere.\n\
                             It works well together with cloud services like Dropbox, iCloud, OneDrive and others.\n\
                             <br /><br />\n\
                             Unlike some other file-system overlay solutions,\n\
                             it does not expose the directory structure,\n\
                             the number of files nor the file sizes\n\
                             through the encrypted data format.\n\
                             <br /><br />\n\
                             One important thing to note is that,\n\
                             while CryFS is considered safe,\n\
                             there is no independent security audit\n\
                             which confirms this."))},
                  step{passwordChooser()},
                  step{directoryPairChooser(DirectoryPairChooserWidget::AutoFillPaths | DirectoryPairChooserWidget::ShowDevicePicker
                                            | DirectoryPairChooserWidget::ShowMountPointPicker | DirectoryPairChooserWidget::RequireEmptyDevice
                                            | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{cryfsCypherChooser(), activitiesChooser(), offlineOnlyChooser()}}},

                {"gocryptfs" / i18n("gocryptfs"),
                 {step{notice("gocryptfs-message", i18n("<b>Security notice:</b>\n\
                             Gocryptfs encrypts your files, so you can safely store them anywhere.\n\
                             It works well together with cloud services like Dropbox, iCloud, OneDrive and others.\n\
                             <br /><br />\n\
                             A threat model for gocryptfs is provided by the author at \
                             <a href='https://nuetzlich.net/gocryptfs/threat_model'>nuetzlich.net/gocryptfs/threat_model</a>. \
                             <br /><br />\n\
                             According to a security audit performed in 2017 by Taylor Hornby (Defuse Security),\n\
                             gocryptfs keeps file contents secret against an adversary that can read and modify the \
                             ciphertext. \
                             <br /><br />\n\
                             See <a href='https://defuse.ca/audits/gocryptfs.htm'>defuse.ca/audits/gocryptfs.htm</a> for more information."))},
                  step{passwordChooser()},
                  step{directoryPairChooser(DirectoryPairChooserWidget::AutoFillPaths | DirectoryPairChooserWidget::ShowDevicePicker
                                            | DirectoryPairChooserWidget::ShowMountPointPicker | DirectoryPairChooserWidget::RequireEmptyDevice
                                            | DirectoryPairChooserWidget::RequireEmptyMountPoint)},
                  step{activitiesChooser(), offlineOnlyChooser()}}}};

    Private(VaultCreationWizard *parent)
        : WBASE(VaultCreationWizard)(parent)
    {
        initBase();
    }

    void finish()
    {
        q->setEnabled(false);

        auto collectedPayload = firstStepModule->fields();
        for (const auto *module : currentStepModules) {
            collectedPayload.insert(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::Device device(collectedPayload[KEY_DEVICE].toString());
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());

        auto vault = new PlasmaVault::Vault(device, q);

        auto future = vault->create(name, mountPoint, collectedPayload);

        auto result = AsynQt::await(future);

        if (result) {
            Q_EMIT q->createdVault(vault);
            q->QDialog::accept();

        } else {
            ui.message->setText(result.error().message());
            ui.message->setMessageType(KMessageWidget::Error);
            ui.message->show();
            vault->scheduleDeletion();
        }

        q->setEnabled(true);
    }
};

VaultCreationWizard::VaultCreationWizard(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
    setWindowTitle(i18nc("@title:window", "Create a New Vault"));
}

VaultCreationWizard::~VaultCreationWizard()
{
}

#include "moc_vaultcreationwizard.cpp"
