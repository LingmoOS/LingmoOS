/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2021 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "NetworkDevice.h"

#include <KLocalizedString>

NetworkDevice::NetworkDevice(const QString &id, const QString &name)
    : SensorObject(id, name, nullptr)
{
    m_networkSensor = new KSysGuard::SensorProperty(QStringLiteral("network"), i18nc("@title", "Network Name"), this);
    m_networkSensor->setShortName(i18nc("@title Short of Network Name", "Name"));
    m_networkSensor->setPrefix(name);

    m_signalSensor = new KSysGuard::SensorProperty(QStringLiteral("signal"), i18nc("@title", "Signal Strength"), 0, this);
    m_signalSensor->setShortName(i18nc("@title Short of Signal Strength", "Signal"));
    m_signalSensor->setUnit(KSysGuard::UnitPercent);
    m_signalSensor->setMin(0);
    m_signalSensor->setMax(100);
    m_signalSensor->setPrefix(name);

    m_ipv4Sensor = new KSysGuard::SensorProperty(QStringLiteral("ipv4address"), i18nc("@title", "IPv4 Address"), this);
    m_ipv4Sensor->setShortName(i18nc("@title Short of IPv4 Address", "IPv4"));
    m_ipv4Sensor->setPrefix(name);

    m_ipv4GatewaySensor = new KSysGuard::SensorProperty(QStringLiteral("ipv4gateway"), i18nc("@title", "IPv4 Gateway"), this);
    m_ipv4GatewaySensor->setShortName(i18nc("@title Short of IPv4 Gateway", "IPv4 Gateway"));
    m_ipv4GatewaySensor->setPrefix(name);

    m_ipv4SubnetMaskSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv4subnet"), i18nc("@title", "IPv4 Subnet Mask"), this);
    m_ipv4SubnetMaskSensor->setShortName(i18nc("@title Short of IPv4 Subnet Mask", "IPv4 Subnet Mask"));
    m_ipv4SubnetMaskSensor->setPrefix(name);

    m_ipv4WithPrefixLengthSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv4withPrefixLength"), i18nc("@title", "IPv4 with Prefix Length"), this);
    m_ipv4WithPrefixLengthSensor->setShortName(i18nc("@title Short of IPv4 Prefix Length", "IPv4"));
    m_ipv4WithPrefixLengthSensor->setPrefix(name);

    m_ipv4DNSSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv4dns"), i18nc("@title", "IPv4 DNS"), this);
    m_ipv4DNSSensor->setShortName(i18nc("@title Short of IPv4 DNS", "IPv4 DNS"));
    m_ipv4DNSSensor->setPrefix(name);

    m_ipv6Sensor = new KSysGuard::SensorProperty(QStringLiteral("ipv6address"), i18nc("@title", "IPv6 Address"), this);
    m_ipv6Sensor->setShortName(i18nc("@title Short of IPv6 Address", "IPv6"));
    m_ipv6Sensor->setPrefix(name);

    m_ipv6GatewaySensor = new KSysGuard::SensorProperty(QStringLiteral("ipv6gateway"), i18nc("@title", "IPv6 Gateway"), this);
    m_ipv6GatewaySensor->setShortName(i18nc("@title Short of IPv6 Gateway", "IPv6 Gateway"));
    m_ipv6GatewaySensor->setPrefix(name);

    m_ipv6SubnetMaskSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv6subnet"), i18nc("@title", "IPv6 Subnet Mask"), this);
    m_ipv6SubnetMaskSensor->setShortName(i18nc("@title Short of IPv6 Subnet Mask", "IPv6 Subnet Mask"));
    m_ipv6SubnetMaskSensor->setPrefix(name);

    m_ipv6WithPrefixLengthSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv6withPrefixLength"), i18nc("@title", "IPv6 with Prefix Length"), this);
    m_ipv6WithPrefixLengthSensor->setShortName(i18nc("@title Short of IPv6 Prefix Length", "IPv6"));
    m_ipv6WithPrefixLengthSensor->setPrefix(name);

    m_ipv6DNSSensor = new KSysGuard::SensorProperty(QStringLiteral("ipv6dns"), i18nc("@title", "IPv6 DNS"), this);
    m_ipv6DNSSensor->setShortName(i18nc("@title Short of IPv6 DNS", "IPv6 DNS"));
    m_ipv6DNSSensor->setPrefix(name);

    m_downloadSensor = new KSysGuard::SensorProperty(QStringLiteral("download"), i18nc("@title", "Download Rate"), 0, this);
    m_downloadSensor->setShortName(i18nc("@title Short for Download Rate", "Download"));
    m_downloadSensor->setUnit(KSysGuard::UnitByteRate);
    m_downloadSensor->setPrefix(name);

    m_uploadSensor = new KSysGuard::SensorProperty(QStringLiteral("upload"), i18nc("@title", "Upload Rate"), 0, this);
    m_uploadSensor->setShortName(i18nc("@title Short for Upload Rate", "Upload"));
    m_uploadSensor->setUnit(KSysGuard::UnitByteRate);
    m_uploadSensor->setPrefix(name);

    m_downloadBitsSensor = new KSysGuard::SensorProperty(QStringLiteral("downloadBits"), i18nc("@title", "Download Rate"), 0, this);
    m_downloadBitsSensor->setShortName(i18nc("@title Short for Download Rate", "Download"));
    m_downloadBitsSensor->setUnit(KSysGuard::UnitBitRate);
    m_downloadBitsSensor->setPrefix(name);

    m_uploadBitsSensor = new KSysGuard::SensorProperty(QStringLiteral("uploadBits"), i18nc("@title", "Upload Rate"), 0, this);
    m_uploadBitsSensor->setShortName(i18nc("@title Short for Upload Rate", "Upload"));
    m_uploadBitsSensor->setUnit(KSysGuard::UnitBitRate);
    m_uploadBitsSensor->setPrefix(name);

    m_totalDownloadSensor = new KSysGuard::SensorProperty(QStringLiteral("totalDownload"), i18nc("@title", "Total Downloaded"), this);
    m_totalDownloadSensor->setShortName(i18nc("@title Short for Total Downloaded", "Downloaded"));
    m_totalDownloadSensor->setUnit(KSysGuard::UnitByte);
    m_totalDownloadSensor->setPrefix(name);

    m_totalUploadSensor = new KSysGuard::SensorProperty(QStringLiteral("totalUpload"), i18nc("@title", "Total Uploaded"), this);
    m_totalUploadSensor->setShortName(i18nc("@title Short for Total Uploaded", "Uploaded"));
    m_totalUploadSensor->setUnit(KSysGuard::UnitByte);
    m_totalDownloadSensor->setPrefix(name);
}

#include "moc_NetworkDevice.cpp"
