/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "device.h"
#include "dbushelper.h"
#include "deviceinterface.h"
#include "libkbolt_debug.h"

#include <stdexcept>

using namespace Bolt;

using DeviceInterface = org::freedesktop::bolt1::Device;

class DBusException : public std::runtime_error
{
public:
    DBusException(const QString &what)
        : std::runtime_error(what.toStdString())
    {
    }
};

Device::Device(QObject *parent)
    : QObject(parent)
{
}

Device::Device(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
    , mInterface(std::make_unique<DeviceInterface>(DBusHelper::serviceName(), path.path(), DBusHelper::connection()))
    , mDBusPath(path)
{
    if (!mInterface->isValid()) {
        throw DBusException(QStringLiteral("Failed to obtain DBus interface for device %1: %2").arg(path.path(), mInterface->lastError().message()));
    }

    // cache UID in case the we still need to identify the device, even if it's
    // gone on DBus
    mUid = mInterface->uid();
}

Device::~Device() = default;

QSharedPointer<Device> Device::create(const QDBusObjectPath &path)
{
    try {
        return QSharedPointer<Device>::create(path);
    } catch (const DBusException &e) {
        qCWarning(log_libkbolt, "%s", e.what());
        return {};
    }
}

QDBusObjectPath Device::dbusPath() const
{
    return mDBusPath;
}

QString Device::uid() const
{
    return mUid;
}

QString Device::name() const
{
    return mInterface->name();
}

QString Device::vendor() const
{
    return mInterface->vendor();
}

Type Device::type() const
{
    const auto val = mInterface->type();
    return val.isEmpty() ? Type::Unknown : typeFromString(val);
}

Status Device::status() const
{
    if (mStatusOverride == Status::Unknown) {
        const auto val = mInterface->status();
        return val.isEmpty() ? Status::Unknown : statusFromString(val);
    } else {
        return mStatusOverride;
    }
}

void Device::setStatusOverride(Status status)
{
    if (mStatusOverride != status) {
        mStatusOverride = status;
        Q_EMIT statusChanged(status);
    }
}

void Device::clearStatusOverride()
{
    setStatusOverride(Status::Unknown);
}

AuthFlags Device::authFlags() const
{
    const auto val = mInterface->authFlags();
    return val.isEmpty() ? Auth::None : authFlagsFromString(val);
}

QString Device::parent() const
{
    return mInterface->parentUid();
}

QString Device::sysfsPath() const
{
    return mInterface->sysfsPath();
}

QDateTime Device::connectTime() const
{
    const auto val = mInterface->connectTime();
    return val == 0 ? QDateTime() : QDateTime::fromSecsSinceEpoch(val);
}

QDateTime Device::authorizeTime() const
{
    const auto val = mInterface->authorizeTime();
    return val == 0 ? QDateTime() : QDateTime::fromSecsSinceEpoch(val);
}

bool Device::stored() const
{
    return mInterface ? mInterface->stored() : false;
}

Policy Device::policy() const
{
    const auto val = mInterface->policy();
    return val.isEmpty() ? Policy::Unknown : policyFromString(val);
}

KeyState Device::keyState() const
{
    const auto val = mInterface->key();
    return val.isEmpty() ? KeyState::Unknown : keyStateFromString(val);
}

QDateTime Device::storeTime() const
{
    const auto val = mInterface->storeTime();
    return val == 0 ? QDateTime() : QDateTime::fromSecsSinceEpoch(val);
}

QString Device::label() const
{
    return mInterface->label();
}

void Device::authorize(AuthFlags authFlags, std::function<void()> successCb, std::function<void(const QString &)> errorCb)
{
    qCDebug(log_libkbolt, "Authorizing device %s with auth flags %s", qUtf8Printable(mUid), qUtf8Printable(authFlagsToString(authFlags)));

    setStatusOverride(Status::Authorizing);
    DBusHelper::call<QString>(
        mInterface.get(),
        QStringLiteral("Authorize"),
        authFlagsToString(authFlags),
        [this, cb = std::move(successCb)]() {
            qCDebug(log_libkbolt, "Device %s was successfully authorized", qUtf8Printable(mUid));
            clearStatusOverride();
            if (cb) {
                cb();
            }
        },
        [this, cb = std::move(errorCb)](const QString &error) {
            qCWarning(log_libkbolt, "Failed to authorize device %s: %s", qUtf8Printable(mUid), qUtf8Printable(error));
            setStatusOverride(Status::AuthError);
            if (cb) {
                cb(error);
            }
        },
        this);
}

#include "moc_device.cpp"
