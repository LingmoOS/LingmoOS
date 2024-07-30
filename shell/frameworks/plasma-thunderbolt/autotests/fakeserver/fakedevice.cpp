/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fakedevice.h"
#include "fakedeviceadaptor.h"

#include <QDBusConnection>
#include <QDBusError>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

FakeDevice::FakeDevice(const QString &uid, QObject *parent)
    : QObject(parent)
    , mUid(uid)
{
    new FakeDeviceAdaptor(this);
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(dbusPath().path(), this)) {
        throw FakeDeviceException(QStringLiteral("Failed to register device %1 to DBus: %2").arg(mUid, bus.lastError().message()));
    }
}

FakeDevice::FakeDevice(const QJsonObject &json, QObject *parent)
    : QObject(parent)
    , mUid(json[QStringLiteral("Uid")].toString())
    , mName(json[QStringLiteral("Name")].toString())
    , mVendor(json[QStringLiteral("Vendor")].toString())
    , mType(json[QStringLiteral("Type")].toString())
    , mStatus(json[QStringLiteral("Status")].toString())
    , mAuthFlags(json[QStringLiteral("AuthFlags")].toString())
    , mParent(json[QStringLiteral("Parent")].toString())
    , mSysfsPath(json[QStringLiteral("SysfsPath")].toString())
    , mPolicy(json[QStringLiteral("Policy")].toString())
    , mKey(json[QStringLiteral("Key")].toString())
    , mLabel(json[QStringLiteral("Label")].toString())
    , mConnectTime(json[QStringLiteral("ConnectTime")].toVariant().value<quint64>())
    , mAuthorizeTime(json[QStringLiteral("AuthorizeTime")].toVariant().value<quint64>())
    , mStoreTime(json[QStringLiteral("StoreTime")].toVariant().value<quint64>())
    , mStored(json[QStringLiteral("Stored")].toBool())
{
    new FakeDeviceAdaptor(this);
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(dbusPath().path(), this)) {
        throw FakeDeviceException(QStringLiteral("Failed to register device %1 to DBus: %2").arg(mUid, bus.lastError().message()));
    }
}

FakeDevice::~FakeDevice()
{
    QDBusConnection::sessionBus().unregisterObject(dbusPath().path());
}

QDBusObjectPath FakeDevice::dbusPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/freedesktop/bolt/devices/%1").arg(QString(mUid).replace(QLatin1Char('-'), QLatin1Char('_'))));
}

QString FakeDevice::uid() const
{
    return mUid;
}

QString FakeDevice::name() const
{
    return mName;
}

void FakeDevice::setName(const QString &name)
{
    mName = name;
}

QString FakeDevice::vendor() const
{
    return mVendor;
}

void FakeDevice::setVendor(const QString &vendor)
{
    mVendor = vendor;
}

QString FakeDevice::type() const
{
    return mType;
}

void FakeDevice::setType(const QString &type)
{
    mType = type;
}

QString FakeDevice::status() const
{
    return mStatus;
}

void FakeDevice::setStatus(const QString &status)
{
    mStatus = status;
}

QString FakeDevice::authFlags() const
{
    return mAuthFlags;
}

void FakeDevice::setAuthFlags(const QString &authFlags)
{
    mAuthFlags = authFlags;
}

QString FakeDevice::parent() const
{
    return mParent;
}

void FakeDevice::setParent(const QString &parent)
{
    mParent = parent;
}

QString FakeDevice::sysfsPath() const
{
    return mSysfsPath;
}

void FakeDevice::setSysfsPath(const QString &sysfsPath)
{
    mSysfsPath = sysfsPath;
}

bool FakeDevice::stored() const
{
    return mStored;
}

void FakeDevice::setStored(bool stored)
{
    mStored = stored;
}

QString FakeDevice::policy() const
{
    return mPolicy;
}

void FakeDevice::setPolicy(const QString &policy)
{
    mPolicy = policy;
}

QString FakeDevice::key() const
{
    return mKey;
}

void FakeDevice::setKey(const QString &key)
{
    mKey = key;
}

QString FakeDevice::label() const
{
    return mLabel;
}

void FakeDevice::setLabel(const QString &label)
{
    mLabel = label;
    Q_EMIT labelChanged(label);
}

quint64 FakeDevice::connectTime() const
{
    return mConnectTime;
}

void FakeDevice::setConnectTime(quint64 connectTime)
{
    mConnectTime = connectTime;
}

quint64 FakeDevice::authorizeTime() const
{
    return mAuthorizeTime;
}

void FakeDevice::setAuthorizeTime(quint64 authorizeTime)
{
    mAuthorizeTime = authorizeTime;
}

quint64 FakeDevice::storeTime() const
{
    return mStoreTime;
}

void FakeDevice::setStoreTime(quint64 storeTime)
{
    mStoreTime = storeTime;
}

void FakeDevice::Authorize(const QString &flags)
{
    std::this_thread::sleep_for(1s); // simulate this operation taking time
    mAuthFlags = flags;
    mStatus = QLatin1String("authorized");
}

#include "moc_fakedevice.cpp"
