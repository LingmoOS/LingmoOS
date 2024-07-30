/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FAKEDEVICE_H
#define FAKEDEVICE_H

#include <QDBusObjectPath>
#include <QJsonObject>
#include <QObject>

class FakeDeviceException : public std::runtime_error
{
public:
    FakeDeviceException(const QString &what)
        : std::runtime_error(what.toStdString())
    {
    }
};

class FakeDevice : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.bolt1.Device")

    Q_PROPERTY(QString Uid READ uid CONSTANT)
    Q_PROPERTY(QString Name READ name CONSTANT)
    Q_PROPERTY(QString Vendor READ vendor CONSTANT)
    Q_PROPERTY(QString Type READ type CONSTANT)
    Q_PROPERTY(QString Status READ status CONSTANT)
    Q_PROPERTY(QString AuthFlags READ authFlags CONSTANT)
    Q_PROPERTY(QString Parent READ parent CONSTANT)
    Q_PROPERTY(QString SysfsPath READ sysfsPath CONSTANT)
    Q_PROPERTY(bool Stored READ stored CONSTANT)
    Q_PROPERTY(QString Policy READ policy CONSTANT)
    Q_PROPERTY(QString Key READ key CONSTANT)
    Q_PROPERTY(QString Label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(quint64 ConnectTime READ connectTime CONSTANT)
    Q_PROPERTY(quint64 AuthorizeTime READ authorizeTime CONSTANT)
    Q_PROPERTY(quint64 StoreTime READ storeTime CONSTANT)
public:
    explicit FakeDevice(const QString &uid, QObject *parent = nullptr);
    explicit FakeDevice(const QJsonObject &json, QObject *parent = nullptr);
    ~FakeDevice() override;

    QDBusObjectPath dbusPath() const;

    QString uid() const;
    QString name() const;
    void setName(const QString &name);
    QString vendor() const;
    void setVendor(const QString &vendor);
    QString type() const;
    void setType(const QString &type);
    QString status() const;
    void setStatus(const QString &status);
    QString authFlags() const;
    void setAuthFlags(const QString &authFlags);
    QString parent() const;
    void setParent(const QString &parent);
    QString sysfsPath() const;
    void setSysfsPath(const QString &sysfsPath);
    bool stored() const;
    void setStored(bool stored);
    QString policy() const;
    void setPolicy(const QString &policy);
    QString key() const;
    void setKey(const QString &key);
    QString label() const;
    void setLabel(const QString &label);
    quint64 connectTime() const;
    void setConnectTime(quint64 connectTime);
    quint64 authorizeTime() const;
    void setAuthorizeTime(quint64 authorizeTime);
    quint64 storeTime() const;
    void setStoreTime(quint64 storeTime);

public Q_SLOTS:
    void Authorize(const QString &flags);

Q_SIGNALS:
    void labelChanged(const QString &label);

private:
    QDBusObjectPath mDBusPath;

    QString mUid;
    QString mName;
    QString mVendor;
    QString mType;
    QString mStatus = QStringLiteral("unknown");
    QString mAuthFlags = QStringLiteral("none");
    QString mParent;
    QString mSysfsPath;
    QString mPolicy = QStringLiteral("unknown");
    QString mKey;
    QString mLabel;
    quint64 mConnectTime = 0;
    quint64 mAuthorizeTime = 0;
    quint64 mStoreTime = 0;
    bool mStored = false;
};

#endif
