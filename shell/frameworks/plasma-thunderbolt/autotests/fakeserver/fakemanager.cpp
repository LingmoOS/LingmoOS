/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fakemanager.h"
#include "fakedevice.h"
#include "fakemanageradaptor.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QJsonArray>

#include <QDebug>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace
{
static const QString kManagerDBusPath = QStringLiteral("/org/freedesktop/bolt");
}

FakeManager::FakeManager(const QJsonObject &json, QObject *parent)
    : QObject(parent)
    , mProbing(json[QStringLiteral("Probing")].toBool())
    , mDefaultPolicy(json[QStringLiteral("DefaultPolicy")].toString())
    , mSecurityLevel(json[QStringLiteral("SecurityLevel")].toString())
    , mAuthMode(json[QStringLiteral("AuthMode")].toString())
{
    new FakeManagerAdaptor(this);
    if (!QDBusConnection::sessionBus().registerObject(kManagerDBusPath, this)) {
        throw FakeManagerException(QStringLiteral("Failed to register FakeManager to DBus: %1").arg(QDBusConnection::sessionBus().lastError().message()));
    }

    const auto jsonDevices = json[QStringLiteral("Devices")].toArray();
    for (const auto &jsonDevice : jsonDevices) {
        auto device = std::make_unique<FakeDevice>(jsonDevice.toObject(), this);
        mDevices.emplace(device->uid(), std::move(device));
    }
}

FakeManager::FakeManager(QObject *parent)
    : QObject(parent)
{
    new FakeManagerAdaptor(this);
    if (!QDBusConnection::sessionBus().registerObject(kManagerDBusPath, this)) {
        throw FakeManagerException(QStringLiteral("Failed to register FakeManager to DBus: %1").arg(QDBusConnection::sessionBus().lastError().message()));
    }
}

FakeManager::~FakeManager()
{
    QDBusConnection::sessionBus().unregisterObject(kManagerDBusPath);
}

FakeDevice *FakeManager::addDevice(std::unique_ptr<FakeDevice> device)
{
    const auto it = mDevices.emplace(device->uid(), std::move(device)).first;
    Q_EMIT DeviceAdded(it->second->dbusPath());
    return it->second.get();
}

void FakeManager::removeDevice(const QString &uid)
{
    auto deviceIt = mDevices.find(uid);
    if (deviceIt == mDevices.end()) {
        return;
    }
    auto device = std::move(deviceIt->second);
    mDevices.erase(deviceIt);
    Q_EMIT DeviceRemoved(device->dbusPath());
}

QList<FakeDevice *> FakeManager::devices() const
{
    QList<FakeDevice *> rv;
    rv.reserve(mDevices.size());
    std::transform(mDevices.cbegin(), mDevices.cend(), std::back_inserter(rv), [](const auto &v) {
        return v.second.get();
    });
    return rv;
}

unsigned int FakeManager::version() const
{
    return 1;
}

bool FakeManager::isProbing() const
{
    return mProbing;
}

QString FakeManager::defaultPolicy() const
{
    return mDefaultPolicy;
}

QString FakeManager::securityLevel() const
{
    return mSecurityLevel;
}

QString FakeManager::authMode() const
{
    return mAuthMode;
}

void FakeManager::setAuthMode(const QString &authMode)
{
    qDebug("Manager: authMode changed to %s", qUtf8Printable(authMode));
    mAuthMode = authMode;
    Q_EMIT authModeChanged(authMode);
}

QList<QDBusObjectPath> FakeManager::ListDevices() const
{
    QList<QDBusObjectPath> rv;
    rv.reserve(mDevices.size());
    for (const auto &device : mDevices) {
        rv.push_back(device.second->dbusPath());
    }
    return rv;
}

QDBusObjectPath FakeManager::DeviceByUid(const QString &uid) const
{
    auto device = mDevices.find(uid);
    if (device == mDevices.cend()) {
        return QDBusObjectPath();
    } else {
        return device->second->dbusPath();
    }
}

QDBusObjectPath FakeManager::EnrollDevice(const QString &uid, const QString &policy, const QString &flags)
{
    std::this_thread::sleep_for(1s); // simulate this operation taking time

    auto deviceIt = mDevices.find(uid);
    if (deviceIt == mDevices.end()) {
        return QDBusObjectPath();
    }
    auto &device = deviceIt->second;
    if (policy == QLatin1String("default")) {
        device->setPolicy(defaultPolicy());
    } else {
        device->setPolicy(policy);
    }
    device->setAuthFlags(flags);
    device->setStored(true);
    device->setStatus(QLatin1String("authorized"));

    return device->dbusPath();
}

void FakeManager::ForgetDevice(const QString &uid)
{
    std::this_thread::sleep_for(1s); // simulate this operation taking time

    auto deviceIt = mDevices.find(uid);
    if (deviceIt == mDevices.end()) {
        return;
    }
    auto &device = deviceIt->second;
    device->setStored(false);
    device->setStatus(QLatin1String("connected"));
}

#include "moc_fakemanager.cpp"
