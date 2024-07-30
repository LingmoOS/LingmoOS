/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "service.h"

#include <QDBusObjectPath>
#include <QMessageBox>
#include <QProcess>

#include <PlasmaActivities/Consumer>
#include <KApplicationTrader>
#include <KIO/ApplicationLauncherJob>
#include <KLocalizedString>
#include <KPasswordDialog>
#include <KPluginFactory>
#include <KService>

#include "engine/commandresult.h"
#include "engine/vault.h"

#include "ui/mountdialog.h"
#include "ui/vaultconfigurationdialog.h"
#include "ui/vaultcreationwizard.h"
#include "ui/vaultimportingwizard.h"

#include <functional>

#include <asynqt/operations/listen.h>

#include <config-plasma-vault.h>
#if HAVE_NETWORKMANAGER
#include <NetworkManagerQt/Manager>
#else
namespace NetworkManager
{
bool isNetworkingEnabled()
{
    return true;
}

void setNetworkingEnabled(bool enabled)
{
    Q_UNUSED(enabled);
}
}
#endif

K_PLUGIN_FACTORY_WITH_JSON(PlasmaVaultServiceFactory, "plasmavault.json", registerPlugin<PlasmaVaultService>();)

using namespace PlasmaVault;

using AsynQt::Expected;

class PlasmaVaultService::Private
{
public:
    QHash<Device, Vault *> knownVaults;
    QSet<Device> openVaults;
    KActivities::Consumer kamd;

    struct NetworkingState {
        bool wasNetworkingEnabled;
        QList<QString> devicesInhibittingNetworking;
    };
    // Ideally, this would be std::optional... lovely C++17
    Expected<NetworkingState, int> savedNetworkingState = Expected<NetworkingState, int>::error(0);

    void saveNetworkingState()
    {
        // Ignore the request if we already have a saved state
        if (savedNetworkingState) {
            return;
        }

        savedNetworkingState = Expected<NetworkingState, int>::success(NetworkingState{NetworkManager::isNetworkingEnabled() || true, {}});
    }

    void restoreNetworkingState()
    {
        // Ignore the request if we do not have a state saved
        // or if there are more devices inhibitting networking
        if (!savedNetworkingState || !savedNetworkingState->devicesInhibittingNetworking.isEmpty()) {
            return;
        }

        NetworkManager::setNetworkingEnabled(savedNetworkingState->wasNetworkingEnabled);
    }

    Vault *vaultFor(const QString &device_) const
    {
        const Device device(device_);

        if (!knownVaults.contains(device)) {
            return nullptr;
        }

        return knownVaults[device];
    }
};

PlasmaVaultService::PlasmaVaultService(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
    , d(new Private())
{
    connect(this, &KDEDModule::moduleRegistered, this, &PlasmaVaultService::slotRegistered);

    // Close vaults that don't belong to the current activity
    connect(&d->kamd, &KActivities::Consumer::currentActivityChanged, this, &PlasmaVaultService::onCurrentActivityChanged);

    // When an activity is deleted, remove it from all the vaults
    connect(&d->kamd, &KActivities::Consumer::activityRemoved, this, &PlasmaVaultService::onActivityRemoved);

    // When activities are loaded, remove activities that no longer exist
    // the vaults
    connect(&d->kamd, &KActivities::Consumer::activitiesChanged, this, &PlasmaVaultService::onActivitiesChanged);

    for (const Device &device : Vault::availableDevices()) {
        registerVault(new Vault(device, this));
    }

    onActivitiesChanged(d->kamd.activities());
}

PlasmaVaultService::~PlasmaVaultService()
{
}

PlasmaVault::VaultInfoList PlasmaVaultService::availableDevices() const
{
    PlasmaVault::VaultInfoList result;
    for (const auto &vault : d->knownVaults.values()) {
        result << vault->info();
    }
    return result;
}

void PlasmaVaultService::requestNewVault()
{
    const auto dialog = new VaultCreationWizard();

    connect(dialog, &VaultCreationWizard::createdVault, this, &PlasmaVaultService::registerVault);

    dialog->show();
}

void PlasmaVaultService::requestImportVault()
{
    const auto dialog = new VaultImportingWizard();

    connect(dialog, &VaultImportingWizard::importedVault, this, &PlasmaVaultService::registerVault);

    dialog->show();
}

void PlasmaVaultService::slotRegistered(const QDBusObjectPath &path)
{
    if (path.path() == QLatin1String("/modules/plasmavault")) {
        Q_EMIT registered();
    }
}

void PlasmaVaultService::registerVault(Vault *vault)
{
    if (!vault->isValid()) {
        qWarning() << "Warning: Trying to register an invalid vault: " << vault->device().data();
        return;
    }

    if (d->knownVaults.contains(vault->device())) {
        qWarning() << "Warning: This one is already registered: " << vault->device().data();
        return;
    }

    vault->setParent(this);

    d->knownVaults[vault->device()] = vault;

    connect(vault, &Vault::statusChanged, this, &PlasmaVaultService::onVaultStatusChanged);
    connect(vault, &Vault::messageChanged, this, &PlasmaVaultService::onVaultMessageChanged);
    connect(vault, &Vault::infoChanged, this, &PlasmaVaultService::onVaultInfoChanged);

    Q_EMIT vaultAdded(vault->info());

    if (vault->status() == VaultInfo::Opened) {
        d->openVaults << vault->device();
    }
}

void PlasmaVaultService::forgetVault(Vault *vault)
{
    // Can not be open
    // d->openVaults.remove(vault.device());
    // and therefore can not inhibit networking
    // ... d->savedNetworkingState ...

    Q_EMIT vaultRemoved(vault->device().data());

    d->knownVaults.remove(vault->device());
    vault->deleteLater();
}

void PlasmaVaultService::onVaultStatusChanged(VaultInfo::Status status)
{
    const auto vault = static_cast<Vault *>(sender());

    if (status == VaultInfo::Dismantled) {
        forgetVault(vault);

    } else if (status == VaultInfo::Opened) {
        d->openVaults << vault->device();
        if (d->openVaults.size() == 1) {
            Q_EMIT hasOpenVaultsChanged(true);
        }

    } else {
        d->openVaults.remove(vault->device());
        if (d->openVaults.isEmpty()) {
            Q_EMIT hasOpenVaultsChanged(false);
        }
    }

    if (vault->isOfflineOnly()) {
        d->saveNetworkingState();
        auto &devicesInhibittingNetworking = d->savedNetworkingState->devicesInhibittingNetworking;

        // We need to check whether this vault
        // should be added or removed from the
        // inhibitors list
        const bool alreadyInhibiting = devicesInhibittingNetworking.contains(vault->device().data());

        if (status == VaultInfo::Opened && !alreadyInhibiting) {
            auto deviceOpeningHandle = "{opening}" + vault->device().data();
            devicesInhibittingNetworking.removeAll(deviceOpeningHandle);
            devicesInhibittingNetworking << vault->device().data();
        }

        if (status != VaultInfo::Opened && alreadyInhibiting) {
            devicesInhibittingNetworking.removeAll(vault->device().data());
        }

        // Now, let's handle the networking part
        if (!devicesInhibittingNetworking.isEmpty()) {
            NetworkManager::setNetworkingEnabled(false);
        }

        d->restoreNetworkingState();
    }

    Q_EMIT vaultChanged(vault->info());
}

void PlasmaVaultService::onVaultInfoChanged()
{
    const auto vault = static_cast<Vault *>(sender());
    Q_EMIT vaultChanged(vault->info());
}

void PlasmaVaultService::onVaultMessageChanged(const QString &message)
{
    Q_UNUSED(message);
    const auto vault = static_cast<Vault *>(sender());
    Q_EMIT vaultChanged(vault->info());
}

template<typename OnAccepted, typename OnRejected>
void showPasswordMountDialog(Vault *vault, OnAccepted onAccepted, OnRejected onRejected)
{
    auto dialog = new MountDialog(vault);

    QObject::connect(dialog, &QDialog::accepted, vault, onAccepted);
    QObject::connect(dialog, &QDialog::rejected, vault, onRejected);

    dialog->open();
}
//^

void PlasmaVaultService::openVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (vault->isOpened())
            return;

        if (vault->isOfflineOnly()) {
            d->saveNetworkingState();

            auto &devicesInhibittingNetworking = d->savedNetworkingState->devicesInhibittingNetworking;
            auto deviceOpeningHandle = "{opening}" + vault->device().data();

            // We need to check whether this vault
            // should be added or removed from the
            // inhibitors list
            const bool alreadyInhibiting = devicesInhibittingNetworking.contains(deviceOpeningHandle);

            if (!alreadyInhibiting) {
                devicesInhibittingNetworking << deviceOpeningHandle;
            }

            NetworkManager::setNetworkingEnabled(false);
        }

        auto stopInhibiting = [this, vault] {
            if (d->savedNetworkingState) {
                auto &devicesInhibittingNetworking = d->savedNetworkingState->devicesInhibittingNetworking;
                auto deviceOpeningHandle = "{opening}" + vault->device().data();
                devicesInhibittingNetworking.removeAll(deviceOpeningHandle);
            }
        };

        showPasswordMountDialog(
            vault,
            [this, vault, stopInhibiting] {
                Q_EMIT vaultChanged(vault->info());
                stopInhibiting();
            },
            [this, vault, stopInhibiting] {
                stopInhibiting();
                if (vault->status() != VaultInfo::Opened) {
                    d->restoreNetworkingState();
                }
            });
    }
}

void PlasmaVaultService::closeVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (!vault->isOpened())
            return;

        vault->close();
    }
}

void PlasmaVaultService::configureVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        const auto dialog = new VaultConfigurationDialog(vault);

        dialog->show();
    }
}

void PlasmaVaultService::forceCloseVault(const QString &device)
{
    if (auto vault = d->vaultFor(device)) {
        if (!vault->isOpened())
            return;

        vault->forceClose();
    }
}

void PlasmaVaultService::openVaultInFileManager(const QString &device)
{
    auto openFileManager = [this](const auto &vault) {
        KService::Ptr service = KApplicationTrader::preferredService(QStringLiteral("inode/directory"));

        // A hack to always open a new dolphin window
        // BUG: 445542
        // https://bugs.kde.org/show_bug.cgi?id=445542
        if (service->desktopEntryName() == QStringLiteral("org.kde.dolphin")) {
            service->setExec(service->exec() + QStringLiteral(" --new-window"));
        }

        auto *job = new KIO::ApplicationLauncherJob(service, this);
        job->setUrls({QUrl::fromLocalFile((QString)vault->mountPoint().data())});
        job->start();
    };

    if (auto vault = d->vaultFor(device)) {
        if (vault->isOpened()) {
            openFileManager(vault);

        } else {
            showPasswordMountDialog(
                vault,
                [this, vault, openFileManager] {
                    Q_EMIT vaultChanged(vault->info());
                    openFileManager(vault);
                },
                [this, vault] {
                    if (vault->status() != VaultInfo::Opened && d->savedNetworkingState) {
                        auto &devicesInhibittingNetworking = d->savedNetworkingState->devicesInhibittingNetworking;
                        devicesInhibittingNetworking.removeAll(vault->device().data());
                        d->restoreNetworkingState();
                    }
                });
        }
    }
}

bool PlasmaVaultService::hasOpenVaults() const
{
    return !d->openVaults.isEmpty();
}

void PlasmaVaultService::closeAllVaults()
{
    for (const auto &device : d->openVaults) {
        closeVault(device.data());
    }
}

void PlasmaVaultService::forceCloseAllVaults()
{
    for (const auto &device : d->openVaults) {
        forceCloseVault(device.data());
    }
}

void PlasmaVaultService::deleteVault(const QString &device, const QString &name)
{
    if (!d->knownVaults.contains(Device(device))) {
        qWarning() << "The specified vault does not exist: " << device;
        return;
    }

    auto vault = d->knownVaults[Device(device)];

    if (vault->status() == VaultInfo::Opened) {
        qWarning() << "Can not delete an open vault: " << device;
        return;
    }

    if (vault->name() != name) {
        qWarning() << "Name is not correct: " << device;
        return;
    }

    AsynQt::onFinished(vault->dismantle({}), [](const auto &future) {
        const auto &result = future.result();
        if (result)
            return;

        const auto &error = result.error();
        if (error.code() != Error::OperationCancelled) {
            QMessageBox::critical(nullptr, i18n("Error deleting vault"), error.message());
        }
    });
}

void PlasmaVaultService::updateStatus()
{
    for (const auto &device : d->knownVaults.keys()) {
        auto vault = d->knownVaults[device];
        vault->updateStatus();
    }
}

void PlasmaVaultService::onActivitiesChanged(const QStringList &knownActivities)
{
    if (knownActivities == QStringList{"00000000-0000-0000-0000-000000000000"})
        return;
    qDebug() << "Known activities:" << knownActivities;

    for (auto *vault : d->knownVaults.values()) {
        auto vaultActivities = vault->activities();
        const auto removedBegin = std::remove_if(vaultActivities.begin(), vaultActivities.end(), [&knownActivities](const QString &vaultActivity) {
            return !knownActivities.contains(vaultActivity);
        });
        if (removedBegin != vaultActivities.end()) {
            vaultActivities.erase(removedBegin, vaultActivities.end());
            vault->setActivities(vaultActivities);
            vault->saveConfiguration();
        }
    }
}

void PlasmaVaultService::onCurrentActivityChanged(const QString &currentActivity)
{
    for (auto *vault : d->knownVaults.values()) {
        const auto vaultActivities = vault->activities();
        if (!vaultActivities.isEmpty() && !vaultActivities.contains(currentActivity)) {
            vault->close();
        }
    }
}

void PlasmaVaultService::onActivityRemoved(const QString &removedActivity)
{
    for (auto *vault : d->knownVaults.values()) {
        auto vaultActivities = vault->activities();
        if (vaultActivities.removeAll(removedActivity) > 0) {
            vault->setActivities(vaultActivities);
            vault->saveConfiguration();
            Q_EMIT vaultChanged(vault->info());
        }
    }
}

#include "service.moc"

#include "moc_service.cpp"
