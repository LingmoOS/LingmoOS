/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <QDBusObjectPath>
#include <QDateTime>
#include <QEnableSharedFromThis>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include <memory>

#include "enum.h"
#include "kbolt_export.h"

class OrgFreedesktopBolt1DeviceInterface;
namespace Bolt
{
class Manager;
class KBOLT_EXPORT Device : public QObject, public QEnableSharedFromThis<Device>
{
    Q_OBJECT

    Q_PROPERTY(QString uid READ uid CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT STORED false)
    Q_PROPERTY(QString vendor READ vendor CONSTANT STORED false)
    Q_PROPERTY(Bolt::Type type READ type CONSTANT STORED false)
    Q_PROPERTY(Bolt::Status status READ status NOTIFY statusChanged STORED false)
    Q_PROPERTY(Bolt::AuthFlags authFlags READ authFlags NOTIFY authFlagsChanged STORED false)
    Q_PROPERTY(QString parent READ parent CONSTANT STORED false)
    Q_PROPERTY(QString sysfsPath READ sysfsPath CONSTANT STORED false)
    Q_PROPERTY(QDateTime connectTime READ connectTime CONSTANT STORED false)
    Q_PROPERTY(QDateTime authorizeTime READ authorizeTime CONSTANT STORED false)
    Q_PROPERTY(bool stored READ stored NOTIFY storedChanged STORED false)
    Q_PROPERTY(Bolt::Policy policy READ policy NOTIFY policyChanged STORED false)
    Q_PROPERTY(Bolt::KeyState keyState READ keyState CONSTANT STORED false)
    Q_PROPERTY(QDateTime storeTime READ storeTime CONSTANT STORED false)
    Q_PROPERTY(QString label READ label CONSTANT STORED false)

    friend class Manager;

public:
    static QSharedPointer<Device> create(const QDBusObjectPath &path);
    explicit Device(QObject *parent = nullptr);
    ~Device() override;

    QString uid() const;
    QString name() const;
    QString vendor() const;
    Type type() const;
    Status status() const;
    AuthFlags authFlags() const;
    QString parent() const;
    QString sysfsPath() const;
    QDateTime connectTime() const;
    QDateTime authorizeTime() const;
    bool stored() const;
    Policy policy() const;
    KeyState keyState() const;
    QDateTime storeTime() const;
    QString label() const;

    QDBusObjectPath dbusPath() const;

    void authorize(Bolt::AuthFlags authFlags, std::function<void()> successCb = {}, std::function<void(const QString &)> errorCb = {});

Q_SIGNALS:
    void statusChanged(Bolt::Status);
    void storedChanged(bool stored);
    void policyChanged(Bolt::Policy policy);
    void authFlagsChanged(Bolt::AuthFlags authFlags);

private:
    template<class T> template<class... Args> friend QSharedPointer<T> QSharedPointer<T>::create(Args &&...);

    Device(const QDBusObjectPath &path, QObject *parent = nullptr);

    void setStatusOverride(Status status);
    void clearStatusOverride();

    std::unique_ptr<OrgFreedesktopBolt1DeviceInterface> mInterface;
    QDBusObjectPath mDBusPath;
    QString mUid;
    Status mStatusOverride = Status::Unknown;
};

} // namespace

#endif
