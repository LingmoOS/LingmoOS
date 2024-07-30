/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <QObject>
#include <QSharedPointer>

#include <functional>
#include <memory>

#include "enum.h"
#include "kbolt_export.h"

class QDBusObjectPath;
class OrgFreedesktopBolt1ManagerInterface;
namespace Bolt
{
class Device;
class KBOLT_EXPORT Manager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isAvailable READ isAvailable CONSTANT)
    Q_PROPERTY(uint version READ version CONSTANT STORED false)
    Q_PROPERTY(bool isProbing READ isProbing CONSTANT STORED false)
    Q_PROPERTY(Bolt::Policy defaultPolicy READ defaultPolicy CONSTANT STORED false)
    Q_PROPERTY(Bolt::Security securityLevel READ securityLevel CONSTANT STORED false)
    Q_PROPERTY(Bolt::AuthMode authMode READ authMode WRITE setAuthMode STORED false NOTIFY authModeChanged)

public:
    explicit Manager(QObject *parent = nullptr);
    ~Manager() override;

    bool isAvailable() const;

    uint version() const;
    bool isProbing() const;
    Policy defaultPolicy() const;
    Security securityLevel() const;
    AuthMode authMode() const;
    void setAuthMode(AuthMode mode);

    /**
     * Updates device authorization and stores it persistently.
     */
    void enrollDevice(const QString &uid,
                      Bolt::Policy policy,
                      Bolt::AuthFlags flags,
                      std::function<void()> successCallback = {},
                      std::function<void(const QString &)> errorCallback = {});
    /**
     * Keeps device authorized but removes it from persistent store.
     *
     * Next time the device is plugged in, it will not be authorized.
     */
    void forgetDevice(const QString &uid, std::function<void()> successCallback = {}, std::function<void(const QString &)> errorCallback = {});

    Q_INVOKABLE QSharedPointer<Bolt::Device> device(const QString &uid) const;
    Q_INVOKABLE QSharedPointer<Bolt::Device> device(const QDBusObjectPath &path) const;
    Q_INVOKABLE QList<QSharedPointer<Bolt::Device>> devices() const;

Q_SIGNALS:
    void deviceAdded(const QSharedPointer<Bolt::Device> &device);
    void deviceRemoved(const QSharedPointer<Bolt::Device> &device);
    void authModeChanged(Bolt::AuthMode authMode);

private:
    QSharedPointer<Device> device(std::function<bool(const QSharedPointer<Device> &)> &&match) const;
    std::unique_ptr<OrgFreedesktopBolt1ManagerInterface> mInterface;

    uint mVersion = 0;
    Policy mPolicy = Policy::Unknown;
    Security mSecurity = Security::Unknown;
    AuthMode mAuthMode = AuthMode::Disabled;
    bool mIsProbing = false;

    QList<QSharedPointer<Device>> mDevices;
};

} // namespace

#endif
