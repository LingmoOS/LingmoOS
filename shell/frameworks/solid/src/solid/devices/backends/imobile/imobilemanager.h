/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IMOBILE_IMOBILEMANAGER_H
#define SOLID_BACKENDS_IMOBILE_IMOBILEMANAGER_H

#include <solid/devices/ifaces/devicemanager.h>

#include <libimobiledevice/libimobiledevice.h>

class QFileSystemWatcher;

namespace Solid
{
namespace Backends
{
namespace IMobile
{
class Manager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    explicit Manager(QObject *parent);
    ~Manager() override;

    QObject *createDevice(const QString &udi) override;
    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;
    QStringList allDevices() override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;
    QString udiPrefix() const override;

    void onDeviceEvent(const idevice_event_t *event);

private:
    void spinUp();
    bool m_spunUp = false;
    QStringList m_deviceUdis;
    std::unique_ptr<QFileSystemWatcher> m_watcher;
};

} // namespace IMobile
} // namespace Backends
} // namespace Solid

#endif // SOLID_BACKENDS_IMOBILE_IMOBILEMANAGER_H
