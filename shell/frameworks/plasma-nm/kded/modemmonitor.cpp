/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemmonitor.h"
#include "plasma_nm_kded.h"

#include <QDBusPendingReply>
#include <QPointer>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Manager>

#include <ModemManager/ModemManager.h>
#include <ModemManagerQt/Manager>
#include <ModemManagerQt/Sim>

#include "pindialog.h"

class ModemMonitorPrivate
{
public:
    QPointer<PinDialog> dialog;
};

ModemMonitor::ModemMonitor(QObject *parent)
    : QObject(parent)
    , d_ptr(new ModemMonitorPrivate)
{
    Q_D(ModemMonitor);
    d->dialog.clear();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-nm"));
    KConfigGroup grp(config, QLatin1String("General"));

    if (grp.isValid()) {
        if (grp.readEntry(QLatin1String("UnlockModemOnDetection"), true)) {
            connect(ModemManager::notifier(), &ModemManager::Notifier::modemAdded, this, &ModemMonitor::unlockModem);
            for (const ModemManager::ModemDevice::Ptr &iface : ModemManager::modemDevices()) {
                unlockModem(iface->uni());
            }
        }
    }
}

ModemMonitor::~ModemMonitor()
{
    delete d_ptr;
}

void ModemMonitor::unlockModem(const QString &modemUni)
{
    Q_D(ModemMonitor);

    ModemManager::Modem::Ptr modem;
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::findModemDevice(modemUni);
    if (modemDevice) {
        modem = modemDevice->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
    } else {
        return;
    }

    connect(modem.data(), &ModemManager::Modem::unlockRequiredChanged, this, &ModemMonitor::requestPin, Qt::UniqueConnection);

    if (d->dialog || (modem && modem->unlockRequired() == MM_MODEM_LOCK_NONE) || (modem && modem->unlockRequired() == MM_MODEM_LOCK_UNKNOWN)) {
        return;
    }

    if (modem) {
        // Using queued invocation to prevent kded stalling here until user enters the pin.
        QMetaObject::invokeMethod(modem.data(), "unlockRequiredChanged", Qt::QueuedConnection, Q_ARG(MMModemLock, modem->unlockRequired()));
    }
}

void ModemMonitor::requestPin(MMModemLock lock)
{
    Q_D(ModemMonitor);
    qCDebug(PLASMA_NM_KDED_LOG) << "unlockRequired == " << lock;
    // Handle just SIM-PIN and SIM-PUK, because some other types may cause problems and they are not also handled by nm-applet
    if (lock == MM_MODEM_LOCK_NONE || lock == MM_MODEM_LOCK_UNKNOWN || (lock != MM_MODEM_LOCK_SIM_PIN && lock != MM_MODEM_LOCK_SIM_PUK)) {
        return;
    }

    auto modem = qobject_cast<ModemManager::Modem *>(sender());
    if (!modem) {
        return;
    }

    if (d->dialog) {
        qCDebug(PLASMA_NM_KDED_LOG) << "PinDialog already running";
        return;
    }

#define MM_DIALOG(lockType, dialogType)                                                                                                                        \
    if (lock == lockType) {                                                                                                                                    \
        d->dialog = QPointer<PinDialog>(new PinDialog(modem, PinDialog::dialogType));                                                                          \
    }

    // clang-format off
    /**/ MM_DIALOG(MM_MODEM_LOCK_SIM_PIN, SimPin)
    else MM_DIALOG(MM_MODEM_LOCK_SIM_PIN2, SimPin2)
    else MM_DIALOG(MM_MODEM_LOCK_SIM_PUK, SimPuk)
    else MM_DIALOG(MM_MODEM_LOCK_SIM_PUK2, SimPuk2)
    else MM_DIALOG(MM_MODEM_LOCK_PH_SP_PIN, ModemServiceProviderPin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_SP_PUK, ModemServiceProviderPuk)
    else MM_DIALOG(MM_MODEM_LOCK_PH_NET_PIN, ModemNetworkPin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_NET_PUK, ModemNetworkPuk)
    else MM_DIALOG(MM_MODEM_LOCK_PH_SIM_PIN, ModemPin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_CORP_PIN, ModemCorporatePin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_CORP_PUK, ModemCorporatePuk)
    else MM_DIALOG(MM_MODEM_LOCK_PH_FSIM_PIN, ModemPhFsimPin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_FSIM_PUK, ModemPhFsimPuk)
    else MM_DIALOG(MM_MODEM_LOCK_PH_NETSUB_PIN, ModemNetworkSubsetPin)
    else MM_DIALOG(MM_MODEM_LOCK_PH_NETSUB_PUK, ModemNetworkSubsetPuk);
    // clang-format on

    if (d->dialog.data()->exec() != QDialog::Accepted) {
        goto OUT;
    }

    qCDebug(PLASMA_NM_KDED_LOG) << "Sending unlock code";

    {
        ModemManager::Sim::Ptr sim;
        ModemManager::ModemDevice::Ptr modemDevice = ModemManager::findModemDevice(modem->uni());
        if (modemDevice && modemDevice->sim()) {
            sim = modemDevice->sim();
        }

        if (!sim) {
            return;
        }

        QDBusPendingCallWatcher *watcher = nullptr;

        PinDialog::Type type = d->dialog.data()->type();

        if (type == PinDialog::SimPin || type == PinDialog::SimPin2 //
            || type == PinDialog::ModemServiceProviderPin || type == PinDialog::ModemNetworkPin //
            || type == PinDialog::ModemPin || type == PinDialog::ModemCorporatePin //
            || type == PinDialog::ModemPhFsimPin || type == PinDialog::ModemNetworkSubsetPin) {
            QDBusPendingCall reply = sim->sendPin(d->dialog.data()->pin());
            watcher = new QDBusPendingCallWatcher(reply, sim.data());
        } else if (type == PinDialog::SimPuk //
                   || type == PinDialog::SimPuk2 || type == PinDialog::ModemServiceProviderPuk //
                   || type == PinDialog::ModemNetworkPuk || type == PinDialog::ModemCorporatePuk //
                   || type == PinDialog::ModemPhFsimPuk || type == PinDialog::ModemNetworkSubsetPuk) {
            QDBusPendingCall reply = sim->sendPuk(d->dialog.data()->puk(), d->dialog.data()->pin());
            watcher = new QDBusPendingCallWatcher(reply, sim.data());
        }

        connect(watcher, &QDBusPendingCallWatcher::finished, this, &ModemMonitor::onSendPinArrived);
    }

OUT:
    if (d->dialog) {
        d->dialog.data()->deleteLater();
    }
    d->dialog.clear();
}

void ModemMonitor::onSendPinArrived(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<> reply = *watcher;

    if (reply.isValid()) {
        // Automatically enabling this for cell phones with expensive data plans is not a good idea.
        // NetworkManager::setWwanEnabled(true);
    } else {
        KMessageBox::error(nullptr,
                           i18nc("Text in GSM PIN/PUK unlock error dialog", "Error unlocking modem: %1", reply.error().message()),
                           i18nc("Title for GSM PIN/PUK unlock error dialog", "PIN/PUK unlock error"));
    }

    watcher->deleteLater();
}

#include "moc_modemmonitor.cpp"
