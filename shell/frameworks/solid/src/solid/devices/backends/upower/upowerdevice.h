/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

#ifndef UPOWERDEVICE_H
#define UPOWERDEVICE_H

#include <ifaces/device.h>
#include <solid/deviceinterface.h>

namespace Solid
{
namespace Backends
{
namespace UPower
{
class UPowerDevice : public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    UPowerDevice(const QString &udi);
    ~UPowerDevice() override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;
    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;
    QString description() const override;
    QStringList emblems() const override;
    QString icon() const override;
    QString product() const override;
    QString vendor() const override;
    QString udi() const override;
    QString parentUdi() const override;

    QVariant prop(const QString &key) const;
    bool propertyExists(const QString &key) const;
    QMap<QString, QVariant> allProperties() const;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);

private Q_SLOTS:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);
    void login1Resuming(bool active);

private:
    QString batteryTechnology() const;
    QString m_udi;
    mutable QVariantMap m_cache;
    mutable QStringList m_negativeCache;
    mutable bool m_cacheComplete = false;

    void checkCache(const QString &key) const;
    void loadCache() const;
};

}
}
}

#endif // UPOWERDEVICE_H
