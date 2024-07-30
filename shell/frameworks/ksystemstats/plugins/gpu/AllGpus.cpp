/*
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 */

#include "AllGpus.h"

#include <KLocalizedString>

#include <systemstats/AggregateSensor.h>

AllGpus::AllGpus(KSysGuard::SensorContainer *parent)
    : SensorObject(QStringLiteral("all"), i18nc("@title", "All GPUs"), parent)
{
    m_usageSensor = new KSysGuard::AggregateSensor(this, QStringLiteral("usage"), i18nc("@title", "All GPUs Usage"), 0);
    m_usageSensor->setShortName(i18nc("@title Short for 'All GPUs Usage'", "Usage"));
    m_usageSensor->setMatchSensors(QRegularExpression{QStringLiteral("^(?!all).*$")}, QStringLiteral("usage"));
    m_usageSensor->setAggregateFunction([this](const QVariant &first, const QVariant &second) {
        auto gpuCount = m_usageSensor->matchCount();
        return QVariant::fromValue(first.toDouble() + (second.toDouble() / gpuCount));
    });
    m_usageSensor->setUnit(KSysGuard::UnitPercent);
    m_usageSensor->setMax(100);

    m_totalVramSensor = new KSysGuard::AggregateSensor(this, QStringLiteral("totalVram"), i18nc("@title", "All GPUs Total Memory"));
    m_totalVramSensor->setShortName(i18nc("@title Short for 'All GPUs Total Memory'", "Total"));
    m_totalVramSensor->setMatchSensors(QRegularExpression{QStringLiteral("^(?!all).*$")}, QStringLiteral("totalVram"));
    m_totalVramSensor->setUnit(KSysGuard::UnitByte);

    m_usedVramSensor = new KSysGuard::AggregateSensor(this, QStringLiteral("usedVram"), i18nc("@title", "All GPUs Used Memory"));
    m_usedVramSensor->setShortName(i18nc("@title Short for 'All GPUs Used Memory'", "Used"));
    m_usedVramSensor->setMatchSensors(QRegularExpression{QStringLiteral("^(?!all).*$")}, QStringLiteral("usedVram"));
    m_usedVramSensor->setUnit(KSysGuard::UnitByte);
    m_usedVramSensor->setMax(m_totalVramSensor);
}

#include "moc_AllGpus.cpp"
