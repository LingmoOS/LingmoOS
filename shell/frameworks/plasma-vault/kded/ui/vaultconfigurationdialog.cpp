/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultconfigurationdialog.h"
#include "ui_vaultconfigurationdialog.h"

#include <QList>
#include <QMap>
#include <QPushButton>
#include <QStackedLayout>
#include <QTabWidget>

#include "dialogdsl.h"
#include "vault.h"

using namespace DialogDsl;
using namespace DialogDsl::operators;

#include "activitieslinkingwidget.h"
#include "backendchooserwidget.h"
#include "cryfscypherchooserwidget.h"
#include "directorychooserwidget.h"
#include "namechooserwidget.h"
#include "noticewidget.h"
#include "offlineonlywidget.h"
#include "passwordchooserwidget.h"
#include "vaultdeletionwidget.h"

using PlasmaVault::Vault;

class VaultConfigurationDialog::Private
{
public:
    VaultConfigurationDialog *const q;
    Vault *vault;

    Ui::VaultConfigurationDialog ui;
    QStackedLayout *layout;

    steps currentSteps;
    QList<DialogDsl::DialogModule *> currentModuleDialogs;
    QSet<DialogDsl::DialogModule *> invalidModules;

    steps defaultSteps{i18n("General") / step{nameChooser(), directoryChooser(DirectoryChooserWidget::RequireEmptyMountPoint)},

                       i18n("Advanced") / step{activitiesChooser(), offlineOnlyChooser()},

                       i18n("Delete") / step{vaultDeletion()}};

    Logic logic{{"encfs" / i18n("EncFS"), defaultSteps},

                {"cryfs" / i18n("CryFS"), defaultSteps},

                {"gocryptfs" / i18n("gocryptfs"), defaultSteps}};

    Private(Vault *vault, VaultConfigurationDialog *parent)
        : q(parent)
        , vault(vault)
    {
        ui.setupUi(parent);
        ui.message->hide();

        layout = new QStackedLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        ui.container->setLayout(layout);

        auto tabs = new QTabWidget();
        layout->addWidget(tabs);

        // Loading the backends
        auto modules = logic[Key(vault->backend().toLatin1())];

        Vault::Payload payload{
            {KEY_DEVICE, QVariant(vault->device().data())},
            {KEY_NAME, QVariant(vault->name())},
            {KEY_MOUNT_POINT, QVariant(vault->mountPoint().data())},
            {KEY_ACTIVITIES, QVariant(vault->activities())},
            {KEY_OFFLINEONLY, QVariant(vault->isOfflineOnly())},
        };

        for (const auto &module : modules) {
            DialogModule *stepWidget = new CompoundDialogModule(module);
            stepWidget->init(payload);
            tabs->addTab(stepWidget, module.title());
            currentModuleDialogs << stepWidget;

            QObject::connect(stepWidget, &DialogModule::isValidChanged, q, [this, stepWidget](bool valid) {
                if (valid) {
                    invalidModules.remove(stepWidget);
                } else {
                    invalidModules << stepWidget;
                }

                ui.buttons->button(QDialogButtonBox::Ok)->setEnabled(invalidModules.isEmpty());
            });

            QObject::connect(stepWidget, &DialogModule::requestCancellation, q, [this] {
                q->reject();
            });
        }
    }

    void setVaultOpened(bool vaultOpened)
    {
        bool configurationEnabled = !vaultOpened;
        ui.buttons->button(QDialogButtonBox::Ok)->setEnabled(configurationEnabled);
        ui.frameUnlockVault->setVisible(!configurationEnabled);
        ui.container->setEnabled(configurationEnabled);
    }

    void saveConfiguration()
    {
        Vault::Payload collectedPayload;
        qDebug() << "Getting the data";
        for (const auto *module : currentModuleDialogs) {
            qDebug() << "Data: " << module->fields();
            collectedPayload.insert(module->fields());
        }

        const auto name = collectedPayload[KEY_NAME].toString();
        const PlasmaVault::MountPoint mountPoint(collectedPayload[KEY_MOUNT_POINT].toString());
        const auto activities = collectedPayload[KEY_ACTIVITIES].toStringList();
        const auto isOfflineOnly = collectedPayload[KEY_OFFLINEONLY].toBool();

        if (name.isEmpty() || mountPoint.isEmpty())
            return;

        vault->setName(name);
        vault->setMountPoint(mountPoint);
        vault->setActivities(activities);
        vault->setIsOfflineOnly(isOfflineOnly);
    }
};

VaultConfigurationDialog::VaultConfigurationDialog(Vault *vault, QWidget *parent)
    : QDialog(parent)
    , d(new Private(vault, this))
{
    setWindowTitle(i18n("Configure"));

    d->setVaultOpened(vault->isOpened());

    connect(d->ui.buttonCloseVault, &QPushButton::clicked, this, [=]() {
        vault->close();
    });

    connect(vault, &Vault::isOpenedChanged, this, [=](bool isOpened) {
        d->setVaultOpened(isOpened);
    });

    connect(d->ui.buttons, &QDialogButtonBox::accepted, this, [=] {
        d->saveConfiguration();
    });
}

VaultConfigurationDialog::~VaultConfigurationDialog()
{
}

#include "moc_vaultconfigurationdialog.cpp"
