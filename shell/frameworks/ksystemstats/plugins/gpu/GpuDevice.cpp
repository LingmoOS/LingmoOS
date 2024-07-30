/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "GpuDevice.h"

#include <KLocalizedString>

GpuDevice::GpuDevice(const QString &id, const QString &name)
    : KSysGuard::SensorObject(id, name)
{
}

void GpuDevice::initialize()
{
    makeSensors();

    m_nameProperty->setName(i18nc("@title", "Name"));
    m_nameProperty->setPrefix(name());
    m_nameProperty->setValue(name());

    m_usageProperty->setName(i18nc("@title", "Usage"));
    m_usageProperty->setPrefix(name());
    m_usageProperty->setMin(0);
    m_usageProperty->setMax(100);
    m_usageProperty->setUnit(KSysGuard::UnitPercent);

    m_totalVramProperty->setName(i18nc("@title", "Total Video Memory"));
    m_totalVramProperty->setPrefix(name());
    m_totalVramProperty->setShortName(i18nc("@title Short for Total Video Memory", "Total"));
    m_totalVramProperty->setUnit(KSysGuard::UnitByte);

    m_usedVramProperty->setName(i18nc("@title", "Video Memory Used"));
    m_usedVramProperty->setPrefix(name());
    m_usedVramProperty->setShortName(i18nc("@title Short for Video Memory Used", "Used"));
    m_usedVramProperty->setMax(m_totalVramProperty);
    m_usedVramProperty->setUnit(KSysGuard::UnitByte);

    m_coreFrequencyProperty->setName(i18nc("@title", "Frequency"));
    m_coreFrequencyProperty->setPrefix(name());
    m_coreFrequencyProperty->setUnit(KSysGuard::UnitMegaHertz);

    m_memoryFrequencyProperty->setName(i18nc("@title", "Memory Frequency"));
    m_memoryFrequencyProperty->setPrefix(name());
    m_memoryFrequencyProperty->setUnit(KSysGuard::UnitMegaHertz);

    m_temperatureProperty->setName(i18nc("@title", "Temperature"));
    m_temperatureProperty->setPrefix(name());
    m_temperatureProperty->setUnit(KSysGuard::UnitCelsius);

    m_powerProperty->setName(i18nc("@title", "Power"));
    m_powerProperty->setPrefix(name());
    m_powerProperty->setUnit(KSysGuard::UnitWatt);
}

void GpuDevice::update()
{
}

void GpuDevice::makeSensors()
{
    m_nameProperty = new KSysGuard::SensorProperty(QStringLiteral("name"), this);
    m_usageProperty = new KSysGuard::SensorProperty(QStringLiteral("usage"), QStringLiteral("usage"), 0, this);
    m_totalVramProperty = new KSysGuard::SensorProperty(QStringLiteral("totalVram"), this);
    m_usedVramProperty = new KSysGuard::SensorProperty(QStringLiteral("usedVram"), this);
    m_coreFrequencyProperty = new KSysGuard::SensorProperty(QStringLiteral("coreFrequency"), QStringLiteral("coreFrequency"), 0, this);
    m_memoryFrequencyProperty = new KSysGuard::SensorProperty(QStringLiteral("memoryFrequency"), QStringLiteral("memoryFrequency"), 0, this);
    m_temperatureProperty = new KSysGuard::SensorProperty(QStringLiteral("temperature"), QStringLiteral("temperature"), 0, this);
    m_powerProperty = new KSysGuard::SensorProperty(QStringLiteral("power"), QStringLiteral("power"), 0, this);
}

#include "moc_GpuDevice.cpp"
