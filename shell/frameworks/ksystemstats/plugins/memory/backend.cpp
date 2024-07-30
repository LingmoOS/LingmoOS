/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "backend.h"

#include <systemstats/AggregateSensor.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>

#include <KLocalizedString>

MemoryBackend::MemoryBackend(KSysGuard::SensorContainer *container)
{
    m_physicalObject = new KSysGuard::SensorObject(QStringLiteral("physical"), i18nc("@title", "Physical Memory"), container);
    m_swapObject = new KSysGuard::SensorObject(QStringLiteral("swap"), i18nc("@title", "Swap Memory"), container);
}

void MemoryBackend::makeSensors()
{
    m_total = new KSysGuard::SensorProperty(QStringLiteral("total"), m_physicalObject);
    m_used = new KSysGuard::SensorProperty(QStringLiteral("used"), m_physicalObject);
    m_free = new KSysGuard::SensorProperty(QStringLiteral("free"), m_physicalObject);
    m_application = new KSysGuard::SensorProperty(QStringLiteral("application"), m_physicalObject);
    m_cache = new KSysGuard::SensorProperty(QStringLiteral("cache"), m_physicalObject);
    m_buffer = new KSysGuard::SensorProperty(QStringLiteral("buffer"), m_physicalObject);

    m_swapTotal = new KSysGuard::SensorProperty(QStringLiteral("total"), m_swapObject);
    m_swapUsed = new KSysGuard::SensorProperty(QStringLiteral("used"), m_swapObject);
    m_swapFree = new KSysGuard::SensorProperty(QStringLiteral("free"), m_swapObject);
}

void MemoryBackend::initSensors()
{
    makeSensors();

    m_total->setName(i18nc("@title", "Total Physical Memory"));
    m_total->setShortName(i18nc("@title, Short for 'Total Physical Memory'", "Total"));
    m_total->setUnit(KSysGuard::UnitByte);
    m_total->setVariantType(QVariant::ULongLong);

    m_used->setName(i18nc("@title", "Used Physical Memory"));
    m_used->setShortName(i18nc("@title, Short for 'Used Physical Memory'", "Used"));
    m_used->setUnit(KSysGuard::UnitByte);
    m_used->setVariantType(QVariant::ULongLong);
    m_used->setMax(m_total);
    auto usedPercentage = new KSysGuard::PercentageSensor(m_physicalObject, QStringLiteral("usedPercent"), i18nc("@title", "Used Physical Memory Percentage"));
    usedPercentage->setShortName(m_used->info().shortName);
    usedPercentage->setBaseSensor(m_used);

    m_free->setName(i18nc("@title", "Free Physical Memory"));
    m_free->setShortName(i18nc("@title, Short for 'Free Physical Memory'", "Free"));
    m_free->setUnit(KSysGuard::UnitByte);
    m_free->setVariantType(QVariant::ULongLong);
    m_free->setMax(m_total);
    auto freePercentage = new KSysGuard::PercentageSensor(m_physicalObject, QStringLiteral("freePercent"), i18nc("@title", "Free Physical Memory Percentage"));
    freePercentage->setShortName(m_free->info().shortName);
    freePercentage->setBaseSensor(m_free);

    m_application->setName(i18nc("@title", "Application Memory"));
    m_application->setShortName(i18nc("@title, Short for 'Application Memory'", "Application"));
    m_application->setUnit(KSysGuard::UnitByte);
    m_application->setVariantType(QVariant::ULongLong);
    m_application->setMax(m_total);
    auto applicationPercentage = new KSysGuard::PercentageSensor(m_physicalObject, QStringLiteral("applicationPercent"), i18nc("@title", "Application Memory Percentage"));
    applicationPercentage->setShortName(m_application->info().shortName);
    applicationPercentage->setBaseSensor(m_application);

    m_cache->setName(i18nc("@title", "Cache Memory"));
    m_cache->setShortName(i18nc("@title, Short for 'Cache Memory'", "Cache"));
    m_cache->setUnit(KSysGuard::UnitByte);
    m_cache->setVariantType(QVariant::ULongLong);
    m_cache->setMax(m_total);
    auto cachePercentage = new KSysGuard::PercentageSensor(m_physicalObject, QStringLiteral("cachePercent"), i18nc("@title", "Cache Memory Percentage"));
    cachePercentage->setShortName(m_cache->info().shortName);
    cachePercentage->setBaseSensor(m_cache);

    m_buffer->setName(i18nc("@title", "Buffer Memory"));
    m_buffer->setShortName(i18nc("@title, Short for 'Buffer Memory'", "Buffer"));
    m_buffer->setDescription(i18n("Amount of memory used for caching disk blocks"));
    m_buffer->setUnit(KSysGuard::UnitByte);
    m_buffer->setVariantType(QVariant::ULongLong);
    m_buffer->setMax(m_total);
    auto bufferPercentage = new KSysGuard::PercentageSensor(m_physicalObject, QStringLiteral("bufferPercent"), i18nc("@title", "Buffer Memory Percentage"));
    bufferPercentage->setShortName(m_buffer->info().shortName);
    bufferPercentage->setBaseSensor(m_buffer);

    m_swapTotal->setName(i18nc("@title", "Total Swap Memory"));
    m_swapTotal->setShortName(i18nc("@title, Short for 'Total Swap Memory'", "Total"));
    m_swapTotal->setUnit(KSysGuard::UnitByte);
    m_swapTotal->setVariantType(QVariant::ULongLong);

    m_swapUsed->setName(i18nc("@title", "Used Swap Memory"));
    m_swapUsed->setShortName(i18nc("@title, Short for 'Used Swap Memory'", "Used"));
    m_swapUsed->setUnit(KSysGuard::UnitByte);
    m_swapUsed->setVariantType(QVariant::ULongLong);
    m_swapUsed->setMax(m_swapTotal);
    auto usedSwapPercentage = new KSysGuard::PercentageSensor(m_swapObject, QStringLiteral("usedPercent"), i18nc("@title", "Used Swap Memory Percentage"));
    usedSwapPercentage->setShortName(m_swapUsed->info().shortName);
    usedSwapPercentage->setBaseSensor(m_swapUsed);

    m_swapFree->setName(i18nc("@title", "Free Swap Memory"));
    m_swapFree->setShortName(i18nc("@title, Short for 'Free Swap Memory'", "Free"));
    m_swapFree->setUnit(KSysGuard::UnitByte);
    m_swapFree->setVariantType(QVariant::ULongLong);
    m_swapFree->setMax(m_swapTotal);
    auto freeSwapPercentage = new KSysGuard::PercentageSensor(m_swapObject, QStringLiteral("freePercent"), i18nc("@title", "Free Swap Memory Percentage"));
    freeSwapPercentage->setShortName(m_swapFree->info().shortName);
    freeSwapPercentage->setBaseSensor(m_swapFree);
}
