/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FAKEMANAGER_H_
#define FAKEMANAGER_H_

#include <QDBusObjectPath>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

#include <map>
#include <memory>

class FakeManagerException : public std::runtime_error
{
public:
    FakeManagerException(const QString &what)
        : std::runtime_error(what.toStdString())
    {
    }
};

class FakeDevice;
class FakeManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.bolt1.Manager")

    Q_PROPERTY(unsigned int Version READ version CONSTANT)
    Q_PROPERTY(bool Probing READ isProbing CONSTANT)
    Q_PROPERTY(QString DefaultPolicy READ defaultPolicy CONSTANT)
    Q_PROPERTY(QString SecurityLevel READ securityLevel CONSTANT)
    Q_PROPERTY(QString AuthMode READ authMode WRITE setAuthMode NOTIFY authModeChanged)

public:
    explicit FakeManager(const QJsonObject &json, QObject *parent = nullptr);
    explicit FakeManager(QObject *parent = nullptr);
    ~FakeManager() override;

    unsigned int version() const;
    bool isProbing() const;
    QString defaultPolicy() const;
    QString securityLevel() const;
    QString authMode() const;
    void setAuthMode(const QString &authMode);

    FakeDevice *addDevice(std::unique_ptr<FakeDevice> device);
    void removeDevice(const QString &uid);
    QList<FakeDevice *> devices() const;

    Q_INVOKABLE QList<QDBusObjectPath> ListDevices() const;
    Q_INVOKABLE QDBusObjectPath DeviceByUid(const QString &uid) const;
    Q_INVOKABLE QDBusObjectPath EnrollDevice(const QString &uid, const QString &policy, const QString &flags);
    Q_INVOKABLE void ForgetDevice(const QString &uid);

Q_SIGNALS:
    void DeviceAdded(const QDBusObjectPath &device);
    void DeviceRemoved(const QDBusObjectPath &device);
    void authModeChanged(const QString &authMode);

private:
    bool mProbing = false;
    QString mDefaultPolicy = QStringLiteral("auto");
    QString mSecurityLevel = QStringLiteral("none");
    QString mAuthMode = QStringLiteral("enabled");

    std::map<QString /*uid*/, std::unique_ptr<FakeDevice>> mDevices;
};

#endif
