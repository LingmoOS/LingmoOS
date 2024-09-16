// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnmutils.h"
#include "dwirelessdevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DWirelessDevicePrivate::DWirelessDevicePrivate(const quint64 id, DWirelessDevice *parent)
    : DDevicePrivate(id, parent)
#ifdef USE_FAKE_INTERFACE
    , m_wireless(new DWirelessDeviceInterface("/com/deepin/FakeNetworkManager/Devices/" + QByteArray::number(id), this))
#else
    , m_wireless(new DWirelessDeviceInterface("/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(id), this))
#endif
{
}

DWirelessDevice::DWirelessDevice(const quint64 id, QObject *parent)
    : DDevice(*new DWirelessDevicePrivate(id, this), parent)
{
    Q_D(const DWirelessDevice);
    connect(d->m_wireless, &DWirelessDeviceInterface::AccessPointsChanged, this, [this](const QList<QDBusObjectPath> &aps) {
        QList<quint64> ret;
        for (const auto &ap : aps)
            ret.append(getIdFromObjectPath(ap));
        Q_EMIT this->accessPointsChanged(ret);
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::HwAddressChanged, this, [this](const QString &address) {
        Q_EMIT this->HwAddressChanged(address.toUtf8());
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::PermHwAddressChanged, this, [this](const QString &address) {
        Q_EMIT this->permHwAddressChanged(address.toUtf8());
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::ModeChanged, this, [this](const quint32 mode) {
        Q_EMIT this->modeChanged(static_cast<NM80211Mode>(mode));
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::BitrateChanged, this, &DWirelessDevice::bitrateChanged);

    connect(d->m_wireless, &DWirelessDeviceInterface::ActiveAccessPointChanged, this, [this](const QDBusObjectPath &path) {
        Q_EMIT this->activeAccessPointChanged(getIdFromObjectPath(path));
    });

    connect(
        d->m_wireless, &DWirelessDeviceInterface::WirelessCapabilitiesChanged, this, [this](const quint32 wirelessCapabilities) {
            Q_EMIT this->wirelessCapabilitiesChanged(static_cast<NMWifiCap>(wirelessCapabilities));
        });

    connect(d->m_wireless, &DWirelessDeviceInterface::LastScanChanged, this, [this](const qint64 time) {
        Q_EMIT this->lastScanChanged(time);
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::AccessPointAdded, this, [this](const QDBusObjectPath &ap) {
        Q_EMIT this->AccessPointAdded(getIdFromObjectPath(ap));
    });

    connect(d->m_wireless, &DWirelessDeviceInterface::AccessPointRemoved, this, [this](const QDBusObjectPath &ap) {
        Q_EMIT this->AccessPointRemoved(getIdFromObjectPath(ap));
    });
}

QList<quint64> DWirelessDevice::accessPoints() const
{
    Q_D(const DWirelessDevice);
    QList<quint64> ret;
    for (const auto &ap : d->m_wireless->accessPoints())
        ret.append(getIdFromObjectPath(ap));
    return ret;
}

QByteArray DWirelessDevice::HwAddress() const
{
    Q_D(const DWirelessDevice);
    return d->m_wireless->HwAddress().toUtf8();
}

QByteArray DWirelessDevice::permHwAddress() const
{
    Q_D(const DWirelessDevice);
    return d->m_wireless->permHwAddress().toUtf8();
}

NM80211Mode DWirelessDevice::mode() const
{
    Q_D(const DWirelessDevice);
    return static_cast<NM80211Mode>(d->m_wireless->mode());
}

quint32 DWirelessDevice::bitrate() const
{
    Q_D(const DWirelessDevice);
    return d->m_wireless->bitrate();
}

quint64 DWirelessDevice::activeAccessPoint() const
{
    Q_D(const DWirelessDevice);
    return getIdFromObjectPath(d->m_wireless->activeAccessPoint());
}

DWirelessDevice::NMWifiCap DWirelessDevice::wirelessCapabilities() const
{
    Q_D(const DWirelessDevice);
    return static_cast<DWirelessDevice::NMWifiCap>(d->m_wireless->wirelessCapabilities());
}

qint64 DWirelessDevice::lastScan() const
{
    Q_D(const DWirelessDevice);
    return d->m_wireless->lastScan();
}

DExpected<void> DWirelessDevice::requestScan(const Config &options) const
{
    Q_D(const DWirelessDevice);
    auto reply = d->m_wireless->requestScan(options);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<QList<quint64>> DWirelessDevice::getAllAccessPoints() const
{
    Q_D(const DWirelessDevice);
    auto reply = d->m_wireless->getAllAccessPoints();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    QList<quint64> ret;
    for (const auto &ap : reply.value())
        ret.append(getIdFromObjectPath(ap));
    return ret;
}

DNETWORKMANAGER_END_NAMESPACE
