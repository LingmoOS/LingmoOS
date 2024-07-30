/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "manager.h"
#include "dbushelper.h"
#include "device.h"
#include "libkbolt_debug.h"
#include "managerinterface.h"

using namespace Bolt;

using ManagerInterface = org::freedesktop::bolt1::Manager;

Manager::Manager(QObject *parent)
    : QObject(parent)
    , mInterface(std::make_unique<ManagerInterface>(DBusHelper::serviceName(), QStringLiteral("/org/freedesktop/bolt"), DBusHelper::connection()))
{
    qDBusRegisterMetaType<QDBusObjectPath>();

    if (!mInterface->isValid()) {
        qCWarning(log_libkbolt, "Failed to connect to Bolt manager DBus interface: %s", qUtf8Printable(mInterface->lastError().message()));
        return;
    }

    connect(mInterface.get(), &ManagerInterface::DeviceAdded, this, [this](const QDBusObjectPath &path) {
        if (auto device = Device::create(path)) {
            mDevices.push_back(device);
            qCDebug(log_libkbolt,
                    "New Thunderbolt device %s (%s) added, status=%s",
                    qUtf8Printable(device->uid()),
                    qUtf8Printable(device->name()),
                    qUtf8Printable(statusToString(device->status())));
            Q_EMIT deviceAdded(device);
        }
    });
    connect(mInterface.get(), &ManagerInterface::DeviceRemoved, this, [this](const QDBusObjectPath &path) {
        if (auto device = this->device(path)) {
            mDevices.removeOne(device);
            qCDebug(log_libkbolt, "Thunderbolt Device %s (%s) removed", qUtf8Printable(device->uid()), qUtf8Printable(device->name()));
            Q_EMIT deviceRemoved(device);
        }
    });

    const auto devicePaths = mInterface->ListDevices().argumentAt<0>();
    for (const auto &devicePath : devicePaths) {
        if (auto device = Device::create(devicePath)) {
            qCDebug(log_libkbolt,
                    "Discovered Thunderbolt device %s (%s), status=%s",
                    qUtf8Printable(device->uid()),
                    qUtf8Printable(device->name()),
                    qUtf8Printable(statusToString(device->status())));
            mDevices.push_back(device);
        }
    }
}

Manager::~Manager() = default;

bool Manager::isAvailable() const
{
    return mInterface.get() && mInterface->isValid();
}

uint Manager::version() const
{
    return mInterface->version();
}

bool Manager::isProbing() const
{
    return mInterface->probing();
}

Policy Manager::defaultPolicy() const
{
    const auto policy = mInterface->defaultPolicy();
    if (!mInterface->isValid() || policy.isEmpty()) {
        return Policy::Unknown;
    }
    return policyFromString(policy);
}

Security Manager::securityLevel() const
{
    const auto level = mInterface->securityLevel();
    if (!mInterface->isValid() || level.isEmpty()) {
        return Security::Unknown;
    }
    return securityFromString(level);
}

AuthMode Manager::authMode() const
{
    const auto mode = mInterface->authMode();
    if (!mInterface->isValid() || mode.isEmpty()) {
        return AuthMode::Disabled;
    }
    return authModeFromString(mode);
}

void Manager::setAuthMode(AuthMode mode)
{
    mInterface->setAuthMode(authModeToString(mode));
    Q_EMIT authModeChanged(mode);
}

QSharedPointer<Device> Manager::device(std::function<bool(const QSharedPointer<Device> &)> &&match) const
{
    auto device = std::find_if(mDevices.cbegin(), mDevices.cend(), std::move(match));
    return device == mDevices.cend() ? QSharedPointer<Device>() : *device;
}

QSharedPointer<Device> Manager::device(const QString &uid) const
{
    return device([uid](const auto &device) {
        return device->uid() == uid;
    });
}

QSharedPointer<Device> Manager::device(const QDBusObjectPath &path) const
{
    return device([path](const auto &device) {
        return device->dbusPath() == path;
    });
}

QList<QSharedPointer<Device>> Manager::devices() const
{
    return mDevices;
}

void Manager::enrollDevice(const QString &uid,
                           Policy policy,
                           AuthFlags authFlags,
                           std::function<void()> successCallback,
                           std::function<void(const QString &)> errorCallback)
{
    qCDebug(log_libkbolt,
            "Enrolling Thunderbolt device %s with policy %s and flags %s",
            qUtf8Printable(uid),
            qUtf8Printable(policyToString(policy)),
            qUtf8Printable(authFlagsToString(authFlags)));

    auto device = this->device(uid);
    if (device) {
        device->setStatusOverride(Status::Authorizing);
    } else {
        qCWarning(log_libkbolt, "Found no matching Thunderbolt device object for uid %s", qUtf8Printable(uid));
    }

    DBusHelper::call<QString, QString, QString>(
        mInterface.get(),
        QStringLiteral("EnrollDevice"),
        uid,
        policyToString(policy),
        authFlagsToString(authFlags),
        [uid, device, policy, authFlags, cb = std::move(successCallback)]() {
            qCDebug(log_libkbolt, "Thunderbolt device %s was successfully enrolled", qUtf8Printable(uid));
            if (device) {
                device->clearStatusOverride();
                Q_EMIT device->storedChanged(true);
                Q_EMIT device->policyChanged(policy);
                Q_EMIT device->authFlagsChanged(authFlags);
            }
            if (cb) {
                cb();
            }
        },
        [uid, device, cb = std::move(errorCallback)](const QString &error) {
            qCWarning(log_libkbolt, "Failed to enroll Thunderbolt device %s: %s", qUtf8Printable(uid), qUtf8Printable(error));
            if (device) {
                device->setStatusOverride(Status::AuthError);
            }
            if (cb) {
                cb(error);
            }
        },
        this);
}

void Manager::forgetDevice(const QString &uid, std::function<void()> successCallback, std::function<void(const QString &)> errorCallback)
{
    qCDebug(log_libkbolt, "Forgetting Thunderbolt device %s", qUtf8Printable(uid));

    DBusHelper::call<QString>(
        mInterface.get(),
        QStringLiteral("ForgetDevice"),
        uid,
        [this, uid, cb = std::move(successCallback)]() {
            qCDebug(log_libkbolt, "Thunderbolt device %s was successfully forgotten", qUtf8Printable(uid));
            if (auto device = this->device(uid)) {
                device->clearStatusOverride();
                Q_EMIT device->storedChanged(false);
                Q_EMIT device->authFlagsChanged(Auth::None);
                Q_EMIT device->policyChanged(Policy::Auto);
            }
            if (cb) {
                cb();
            }
        },
        [this, uid, cb = std::move(errorCallback)](const QString &error) {
            qCWarning(log_libkbolt, "Failed to forget Thunderbolt device %s: %s", qUtf8Printable(uid), qUtf8Printable(error));
            if (auto device = this->device(uid)) {
                device->setStatusOverride(Status::AuthError);
            }
            if (cb) {
                cb(error);
            }
        },
        this);
}

#include "moc_manager.cpp"
