/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kded_bolt.h"
#include "kded_bolt_debug.h"

#include "device.h"

#include <QPointer>

#include <KLocalizedString>
#include <KNotification>

#include <chrono>

using namespace std::chrono_literals;

KDEDBolt::KDEDBolt(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
{
    if (!mManager.isAvailable()) {
        qCInfo(log_kded_bolt, "Couldn't connect to Bolt DBus daemon");
        return;
    }

    mPendingDeviceTimer.setSingleShot(true);
    mPendingDeviceTimer.setInterval(500ms);
    connect(&mPendingDeviceTimer, &QTimer::timeout, this, &KDEDBolt::notify);

    connect(&mManager, &Bolt::Manager::deviceAdded, this, [this](const QSharedPointer<Bolt::Device> &device) {
        // Already authorized, nothing else to do here
        if (device->status() == Bolt::Status::Authorized) {
            return;
        }

        mPendingDevices.append(device);
        mPendingDeviceTimer.start();
    });
    connect(&mManager, &Bolt::Manager::deviceRemoved, this, [this](const QSharedPointer<Bolt::Device> &device) {
        // Check if maybe the device is in pending or currently active
        // notification, remove it if so.
        mPendingDevices.removeOne(device);
        Q_ASSERT(!mPendingDevices.removeOne(device));

        for (auto it = mNotifiedDevices.begin(), end = mNotifiedDevices.end(); it != end; ++it) {
            if (it->contains(device)) {
                auto devices = *it;
                devices.removeOne(device);
                mPendingDevices += devices;
                mPendingDeviceTimer.start();
            }
            it.key()->close();
        }
    });
}

KDEDBolt::~KDEDBolt()
{
}

void KDEDBolt::notify()
{
    auto ntf = KNotification::event(QStringLiteral("unauthorizedDeviceConnected"),
                                    i18n("New Thunderbolt Device Detected"),
                                    mPendingDevices.size() == 1 ? i18n("Unauthorized Thunderbolt device <b>%1</b> was detected. Do you want to authorize it?",
                                                                       mPendingDevices.front()->name())
                                                                : i18np("%1 unauthorized Thunderbolt device was detected. Do you want to authorize it?",
                                                                        "%1 unauthorized Thunderbolt devices were detected. Do you want to authorize them?",
                                                                        mPendingDevices.size()),
                                    /*icon*/ QPixmap{},
                                    KNotification::Persistent,
                                    QStringLiteral("kded_bolt"));
    mNotifiedDevices.insert(ntf, mPendingDevices);

    auto authorizeNowAction = ntf->addAction(i18n("Authorize Now"));
    connect(authorizeNowAction, &KNotificationAction::activated, this, [this, devices = mPendingDevices]() {
        authorizeDevices(sortDevices(devices), Authorize);
    });

    auto authorizePermanentlyAction = ntf->addAction(i18n("Authorize Permanently"));
    connect(authorizePermanentlyAction, &KNotificationAction::activated, this, [this, devices = mPendingDevices]() {
        authorizeDevices(sortDevices(devices), Enroll);
    });

    connect(ntf, &KNotification::closed, this, [this, ntf]() {
        mNotifiedDevices.remove(ntf);
    });

    mPendingDevices.clear();
}

KDEDBolt::BoltDeviceList KDEDBolt::sortDevices(const BoltDeviceList &devices)
{
    QList<QSharedPointer<Bolt::Device>> sorted;
    sorted.reserve(devices.size());

    // Sort the devices so that parents go before their children. Probably
    // fairly inefficient but there's rarely more than a couple of items.
    for (const auto &device : devices) {
        auto child = std::find_if(sorted.begin(), sorted.end(), [device](const auto &d) {
            return d->parent() == device->uid();
        });
        auto parent = std::find_if(sorted.begin(), child, [device](const auto &d) {
            return device->parent() == d->uid();
        });
        if (parent != sorted.end()) {
            ++parent;
        }
        sorted.insert(parent, device);
    }

    return sorted;
}

void KDEDBolt::authorizeDevices(BoltDeviceList devices, AuthMode mode)
{
    if (devices.empty()) {
        return;
    }

    const auto device = devices.takeFirst();

    const auto okCb = [this, devices, mode]() {
        authorizeDevices(std::move(devices), mode);
    };
    const auto errCb = [device](const QString &error) {
        KNotification::event(QStringLiteral("deviceAuthError"),
                             i18n("Thunderbolt Device Authorization Error"),
                             i18n("Failed to authorize Thunderbolt device <b>%1</b>: %2", device->name().toHtmlEscaped(), error),
                             /* icon */ QPixmap{},
                             KNotification::CloseOnTimeout,
                             QStringLiteral("kded_bolt"));
    };
    if (mode == Enroll) {
        mManager.enrollDevice(device->uid(), Bolt::Policy::Auto, Bolt::Auth::Boot | Bolt::Auth::NoKey, okCb, errCb);
    } else {
        device->authorize(Bolt::Auth::Boot | Bolt::Auth::NoKey, okCb, errCb);
    }
}

#include "moc_kded_bolt.cpp"
