/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H
#define SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H

#include <solid/devices/ifaces/devicemanager.h>

class QDomElement;

using namespace Solid::Ifaces;

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice;

/**
 * @brief a Fake manager that read a device list from a XML file.
 * This fake manager is used for unit tests and developers.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class FakeManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT
public:
    FakeManager(QObject *parent, const QString &xmlFile);
    ~FakeManager() override;

    QString udiPrefix() const override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;

    /**
     * Return the list of UDI of all available devices.
     */
    QStringList allDevices() override;

    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;

    QObject *createDevice(const QString &udi) override;
    virtual FakeDevice *findDevice(const QString &udi);

public Q_SLOTS:
    void plug(const QString &udi);
    void unplug(const QString &udi);

private Q_SLOTS:
    /**
     * @internal
     * Parse the XML file that represent the fake machine.
     */
    void parseMachineFile();
    /**
     * @internal
     * Parse a device node and the return the device.
     */
    FakeDevice *parseDeviceElement(const QDomElement &element);

private:
    QStringList findDeviceStringMatch(const QString &key, const QString &value);
    QStringList findDeviceByDeviceInterface(Solid::DeviceInterface::Type type);

    class Private;
    Private *d;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H
