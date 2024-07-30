/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "imobilemanager.h"

#include <QFile>
#include <QFileSystemWatcher>

#include "imobile_debug.h"

#include "../shared/rootdevice.h"
#include "imobile.h"
#include "imobiledevice.h"

using namespace Solid::Backends::IMobile;
using namespace Solid::Backends::Shared;
using namespace Qt::StringLiterals;

namespace
{
constexpr auto VAR_RUN = "/var/run/"_L1;
constexpr auto MUXD_SOCKET = "/var/run/usbmuxd"_L1;
} // namespace

Manager::Manager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , m_watcher(new QFileSystemWatcher)
{
    // Lazy initialize. If usbmuxd isn't running we don't need to do anything yet.
    // This is in part to prevent libusbmuxd from setting up extra inotifies and polling when
    // we know that it won't find anything yet. Works around a bunch of whoopsies.
    // https://github.com/libimobiledevice/libusbmuxd/pull/133
    // https://github.com/libimobiledevice/libusbmuxd/issues/135
    connect(m_watcher.get(), &QFileSystemWatcher::directoryChanged, this, [this](const QString &) {
        if (QFile::exists(MUXD_SOCKET)) {
            spinUp();
        }
    });
    m_watcher->addPath(VAR_RUN);
    if (QFile::exists(MUXD_SOCKET)) {
        spinUp();
    }
}

Manager::~Manager()
{
    if (m_spunUp) {
        idevice_event_unsubscribe();
    }
}

void Manager::spinUp()
{
    if (m_spunUp) {
        return;
    }
    m_spunUp = true;
    // Handing over watching to libusbmuxd. Don't need our watcher anymore.
    // Deleting later because this function gets called by a signal from m_watcher so we mustn't delete it just now.
    m_watcher.release()->deleteLater();

    auto ret = idevice_event_subscribe(
        [](const idevice_event_t *event, void *user_data) {
            // NOTE this is called from a different thread.
            static_cast<Manager *>(user_data)->onDeviceEvent(event);
        },
        this);
    if (ret != IDEVICE_E_SUCCESS) {
        qCWarning(IMOBILE) << "Failed to subscribe to device events";
    }

    char **devices = nullptr;
    int count = 0;

    ret = idevice_get_device_list(&devices, &count);
    if (ret != IDEVICE_E_SUCCESS && ret != IDEVICE_E_NO_DEVICE) {
        qCWarning(IMOBILE) << "Failed to get list of iOS devices" << ret;
        return;
    }

    m_deviceUdis.reserve(count);

    for (int i = 0; i < count; ++i) {
        m_deviceUdis.append(Solid::Backends::IMobile::udiPrefix() + QLatin1Char('/') + QString::fromLatin1(devices[i]));
    }

    if (devices) {
        idevice_device_list_free(devices);
    }
}

QObject *Manager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        RootDevice *root = new RootDevice(udi);
        root->setProduct(tr("iDevice"));
        root->setDescription(tr("iOS devices"));
        root->setIcon("phone-apple-iphone");
        return root;
    }

    if (m_deviceUdis.contains(udi)) {
        return new IMobileDevice(udi);
    }

    return nullptr;
}

QStringList Manager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList devices;

    if (!parentUdi.isEmpty() || type != Solid::DeviceInterface::Unknown) {
        for (const QString &udi : m_deviceUdis) {
            IMobileDevice device(udi);
            if (!device.queryDeviceInterface(type)) {
                continue;
            }

            if (!parentUdi.isEmpty() && device.parentUdi() != parentUdi) {
                continue;
            }

            devices << udi;
        }
    }

    return devices;
}

QStringList Manager::allDevices()
{
    return m_deviceUdis;
}

QSet<Solid::DeviceInterface::Type> Manager::supportedInterfaces() const
{
    return {Solid::DeviceInterface::PortableMediaPlayer};
}

QString Manager::udiPrefix() const
{
    return Solid::Backends::IMobile::udiPrefix();
}

void Manager::onDeviceEvent(const idevice_event_t *event)
{
    const QString udi = Solid::Backends::IMobile::udiPrefix() + QLatin1Char('/') + QString::fromLatin1(event->udid);

    switch (event->event) {
    case IDEVICE_DEVICE_ADD:
        // Post it to the right thread.
        QMetaObject::invokeMethod(this, [this, udi] {
            if (!m_deviceUdis.contains(udi)) {
                m_deviceUdis.append(udi);
                Q_EMIT deviceAdded(udi);
            }
        });
        return;
    case IDEVICE_DEVICE_REMOVE:
        QMetaObject::invokeMethod(this, [this, udi] {
            if (m_deviceUdis.removeOne(udi)) {
                Q_EMIT deviceRemoved(udi);
            }
        });
        return;
#if IMOBILEDEVICE_API >= QT_VERSION_CHECK(1, 3, 0)
    case IDEVICE_DEVICE_PAIRED:
        return;
#endif
    }

    qCDebug(IMOBILE) << "Unhandled device event" << event->event << "for" << event->udid;
}

#include "moc_imobilemanager.cpp"
