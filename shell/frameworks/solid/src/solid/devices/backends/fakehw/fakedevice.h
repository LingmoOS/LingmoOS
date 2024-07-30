/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H

#include <solid/devices/ifaces/device.h>

#include <QMap>
#include <QSharedPointer>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice : public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap);
    FakeDevice(const FakeDevice &dev);
    ~FakeDevice() override;

public Q_SLOTS:
    QString udi() const override;
    QString parentUdi() const override;
    QString vendor() const override;
    QString product() const override;
    QString icon() const override;
    QStringList emblems() const override;
    QString description() const override;

    virtual QVariant property(const QString &key) const;
    virtual QMap<QString, QVariant> allProperties() const;
    virtual bool propertyExists(const QString &key) const;
    virtual bool setProperty(const QString &key, const QVariant &value);
    virtual bool removeProperty(const QString &key);

    virtual bool lock(const QString &reason);
    virtual bool unlock();
    virtual bool isLocked() const;
    virtual QString lockReason() const;

    void setBroken(bool broken);
    bool isBroken();
    void raiseCondition(const QString &condition, const QString &reason);

public:
    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;
    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

private:
    class Private;
    QSharedPointer<Private> d;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H
